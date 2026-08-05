// Implementation of the portable C ABI declared in keylt.h.
//
// Everything in this file is a thin, defensive translation layer:
//   - no C++ type or exception ever crosses an exported boundary,
//   - every handle has exactly one create and one destroy function,
//   - every buffer parameter carries an explicit capacity.
//
// The rest of the library is ordinary C++ and stays hidden behind
// -fvisibility=hidden; only the KEYLT_API functions below are exported.

#include "keylt.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "platform.h"

#include "keyboard.h"
#include "keyboard_lcd.h"
#include "misc.h"
#include "print.h"
#include "usb_functions.h"
#include "usb_io.h"
#include "keyboards/known_keyboards.h"
#include "keyboards/rk84/constants_rk84.h"
#include "keyboards/rk84/key_mappings_rk84.h"
#include "keyboards/sk80/constants_sk80.h"
#include "keyboards/sk80/key_mappings_sk80.h"

#ifndef KEYLT_VERSION_STRING
#define KEYLT_VERSION_STRING "0.0.0-unknown"
#endif

namespace {

// --------------------------------------------------------------------------
// Last-error slot
// --------------------------------------------------------------------------

// Thread-local so that concurrent use of distinct handles does not interleave
// error messages. The pointer handed out by keylt_last_error_message() stays
// valid until the next keylt_* call on the same thread.
std::string& LastError() {
    static thread_local std::string message;
    return message;
}

keylt_status Fail(keylt_status status, const char* what) noexcept {
    try {
        LastError().assign(what ? what : "");
    } catch (...) {
        // A message is a convenience; never let recording one change the result.
        LastError().clear();
    }
    return status;
}

keylt_status Fail(keylt_status status, const std::string& what) noexcept {
    return Fail(status, what.c_str());
}

// --------------------------------------------------------------------------
// Exception firewall
// --------------------------------------------------------------------------

// Runs `fn` with every exception translated into a status code. Exported
// functions delegate to this so that no C++ exception can unwind into foreign
// frames, which would be undefined behaviour.
template <typename Fn>
keylt_status Guard(Fn&& fn) noexcept {
    LastError().clear();
    try {
        return fn();
    } catch (const std::invalid_argument& e) {
        return Fail(KEYLT_ERR_INVALID_ARGUMENT, e.what());
    } catch (const std::out_of_range& e) {
        return Fail(KEYLT_ERR_OUT_OF_RANGE, e.what());
    } catch (const std::logic_error& e) {
        // The library raises logic_error for protocol features a model has not
        // implemented yet.
        return Fail(KEYLT_ERR_UNSUPPORTED, e.what());
    } catch (const std::bad_alloc&) {
        return Fail(KEYLT_ERR_OUT_OF_MEMORY, "out of memory");
    } catch (const std::exception& e) {
        return Fail(KEYLT_ERR_INTERNAL, e.what());
    } catch (...) {
        return Fail(KEYLT_ERR_INTERNAL, "unknown error");
    }
}

// --------------------------------------------------------------------------
// Enum translation
// --------------------------------------------------------------------------

// Deliberately a switch and not a cast: keylt_key_value and keylt::KeyValue
// have opposite orderings.
keylt::KeyValue ToKeyValue(keylt_key_value value) {
    switch (value) {
        case KEYLT_KEY_ON:
            return keylt::kOn;
        case KEYLT_KEY_OFF:
            return keylt::kOff;
    }
    throw std::invalid_argument("unrecognised keylt_key_value: " +
                                std::to_string(static_cast<int>(value)));
}

keylt::KeyboardModel ToKeyboardModel(keylt_model model) {
    switch (model) {
        case KEYLT_MODEL_SK80:
            return keylt::kSK80;
        case KEYLT_MODEL_MK84:
            return keylt::kMK84;
        case KEYLT_MODEL_RK84:
            return keylt::kRK84;
    }
    throw std::invalid_argument("unrecognised keylt_model: " +
                                std::to_string(static_cast<int>(model)));
}

int32_t FromKeyboardModel(keylt::KeyboardModel model) {
    switch (model) {
        case keylt::kSK80:
            return KEYLT_MODEL_SK80;
        case keylt::kMK84:
            return KEYLT_MODEL_MK84;
        case keylt::kRK84:
            return KEYLT_MODEL_RK84;
    }
    throw std::invalid_argument("unmapped keylt::KeyboardModel");
}

// Rejects the models that are named in the enum but have no driver behind them,
// so callers get KEYLT_ERR_UNSUPPORTED instead of a vaguer failure from deeper
// in the library.
void RequireImplementedModel(keylt_model model) {
    if (model == KEYLT_MODEL_MK84) {
        throw std::logic_error("keyboard model MK84 is not implemented");
    }
}

// --------------------------------------------------------------------------
// Struct translation
// --------------------------------------------------------------------------

// Field-by-field rather than a memcpy: the C struct is an independent type, so
// the C++ layout (and its unscoped enum's underlying type) is never assumed.
keylt_keyboard_info ToCInfo(const keylt::KeyboardInfo& info) {
    keylt_keyboard_info out{};
    out.model = FromKeyboardModel(info.keyboard_model);
    out.vid = static_cast<uint16_t>(info.vid);
    out.pid = static_cast<uint16_t>(info.pid);
    out.max_key_id = static_cast<uint8_t>(info.max_key_id);
    out.message_length = static_cast<uint8_t>(info.MESSAGE_LENGTH);
    out.bulk_led_value_messages_count =
        static_cast<uint8_t>(info.BULK_LED_VALUE_MESSAGES_COUNT);

    // strncpy would not guarantee termination for a full-width name.
    const size_t limit = KEYLT_DISPLAY_NAME_CAPACITY - 1;
    size_t length = 0;
    while (length < limit && info.display_name[length] != '\0') {
        out.display_name[length] = info.display_name[length];
        ++length;
    }
    out.display_name[length] = '\0';
    return out;
}

// Shared implementation of the two enumeration entry points. `out_count` is
// always the full total so callers can size a buffer from a probing call.
keylt_status CopyInfos(const std::vector<keylt::KeyboardInfo>& source,
                       keylt_keyboard_info* out_infos, size_t capacity,
                       size_t* out_count) {
    if (out_count == nullptr) {
        throw std::invalid_argument("out_count must not be null");
    }
    *out_count = source.size();

    if (out_infos == nullptr) {
        // A probing call is only an error if the caller claimed to have room.
        if (capacity != 0) {
            throw std::invalid_argument("out_infos is null but capacity is non-zero");
        }
        return source.empty() ? KEYLT_OK
                              : Fail(KEYLT_ERR_BUFFER_TOO_SMALL, "buffer too small");
    }
    if (capacity < source.size()) {
        return Fail(KEYLT_ERR_BUFFER_TOO_SMALL, "buffer too small");
    }

    for (size_t i = 0; i < source.size(); ++i) {
        out_infos[i] = ToCInfo(source[i]);
    }
    return KEYLT_OK;
}

// --------------------------------------------------------------------------
// Key name tables
// --------------------------------------------------------------------------

// A model's key names in a stable, sorted order so that index-based iteration
// is meaningful. The underlying maps are unordered, which gives no usable
// index; sorting once here does.
using KeyTable = std::vector<std::pair<const char*, uint8_t>>;

KeyTable BuildKeyTable(const std::unordered_map<std::string, char>& mappings) {
    KeyTable table;
    table.reserve(mappings.size());
    for (const auto& entry : mappings) {
        // The map has static storage duration, so its keys outlive the process
        // and the borrowed pointers stay valid for the caller.
        table.emplace_back(entry.first.c_str(), static_cast<uint8_t>(entry.second));
    }
    std::sort(table.begin(), table.end(),
              [](const std::pair<const char*, uint8_t>& lhs,
                 const std::pair<const char*, uint8_t>& rhs) {
                  return std::strcmp(lhs.first, rhs.first) < 0;
              });
    return table;
}

const KeyTable& KeyTableFor(keylt_model model) {
    RequireImplementedModel(model);
    switch (model) {
        case KEYLT_MODEL_SK80: {
            static const KeyTable table = BuildKeyTable(sk80::keyname_keyid_mappings);
            return table;
        }
        case KEYLT_MODEL_RK84: {
            static const KeyTable table = BuildKeyTable(rk84::keyname_keyid_mappings);
            return table;
        }
        default:
            break;
    }
    throw std::invalid_argument("unrecognised keylt_model: " +
                                std::to_string(static_cast<int>(model)));
}

// --------------------------------------------------------------------------
// USB transport adapter
// --------------------------------------------------------------------------

// Bridges a caller-supplied keylt_usb_io vtable onto the library's IUsbIO
// interface. The vtable is copied on construction, so the caller's struct need
// not outlive the handle.
class CApiUsbIO final : public keylt::IUsbIO {
public:
    explicit CApiUsbIO(const keylt_usb_io& io) : io_(io) {}

    bool Open(short vid, short pid, const char* control_interface,
              const char* data_interface) override {
        if (io_.open == nullptr) {
            return false;
        }
        return io_.open(io_.user_data, static_cast<uint16_t>(vid),
                        static_cast<uint16_t>(pid), control_interface,
                        data_interface) != 0;
    }

    int SendFeatureReport(const unsigned char* data, size_t length) override {
        if (io_.send_feature_report == nullptr) {
            return -1;
        }
        return io_.send_feature_report(io_.user_data, data, length);
    }

    int GetFeatureReport(unsigned char* buffer, size_t length) override {
        if (io_.get_feature_report == nullptr) {
            return -1;
        }
        return io_.get_feature_report(io_.user_data, buffer, length);
    }

    int WriteData(const unsigned char* data, size_t length) override {
        if (io_.write_data == nullptr) {
            return -1;
        }
        return io_.write_data(io_.user_data, data, length);
    }

    int ReadData(unsigned char* buffer, size_t length, int timeout_ms) override {
        if (io_.read_data == nullptr) {
            return -1;
        }
        return io_.read_data(io_.user_data, buffer, length,
                             static_cast<int32_t>(timeout_ms));
    }

    void Close() override {
        if (io_.close != nullptr) {
            io_.close(io_.user_data);
        }
    }

private:
    keylt_usb_io io_;
};

}  // namespace

// --------------------------------------------------------------------------
// Handle definitions
// --------------------------------------------------------------------------

// Declared opaquely in keylt.h; only this translation unit knows the layout.
struct keylt_keyboard {
    keylt_keyboard(keylt_model model_in, keylt::KeyboardModel cxx_model)
        : model(model_in), keyboard(cxx_model) {}

    keylt_model model;
    keylt::Keyboard keyboard;
};

struct keylt_lcd {
    // Declaration order matters: `lcd` holds borrowed pointers into `io`,
    // `control_interface` and `data_interface`, so those must be constructed
    // first and destroyed last.
    std::unique_ptr<keylt::IUsbIO> io;
    std::string control_interface;
    std::string data_interface;
    keylt::KeyboardLcd lcd;

    keylt_lcd(std::unique_ptr<keylt::IUsbIO> io_in, uint16_t vid, uint16_t pid,
              std::string control, std::string data)
        : io(std::move(io_in)),
          control_interface(std::move(control)),
          data_interface(std::move(data)),
          lcd(io.get(), static_cast<short>(vid), static_cast<short>(pid),
              control_interface.c_str(), data_interface.c_str()) {}
};

// --------------------------------------------------------------------------
// Library information
// --------------------------------------------------------------------------

extern "C" {

KEYLT_API uint32_t KEYLT_CALL keylt_abi_version(void) {
    return KEYLT_ABI_VERSION;
}

KEYLT_API const char* KEYLT_CALL keylt_version_string(void) {
    return KEYLT_VERSION_STRING;
}

// --------------------------------------------------------------------------
// Errors
// --------------------------------------------------------------------------

KEYLT_API const char* KEYLT_CALL keylt_last_error_message(void) {
    return LastError().c_str();
}

KEYLT_API void KEYLT_CALL keylt_clear_last_error(void) {
    LastError().clear();
}

KEYLT_API const char* KEYLT_CALL keylt_status_name(int32_t status) {
    switch (status) {
        case KEYLT_OK:
            return "KEYLT_OK";
        case KEYLT_ERR_INVALID_ARGUMENT:
            return "KEYLT_ERR_INVALID_ARGUMENT";
        case KEYLT_ERR_DEVICE_NOT_FOUND:
            return "KEYLT_ERR_DEVICE_NOT_FOUND";
        case KEYLT_ERR_OUT_OF_RANGE:
            return "KEYLT_ERR_OUT_OF_RANGE";
        case KEYLT_ERR_UNSUPPORTED:
            return "KEYLT_ERR_UNSUPPORTED";
        case KEYLT_ERR_IO:
            return "KEYLT_ERR_IO";
        case KEYLT_ERR_OUT_OF_MEMORY:
            return "KEYLT_ERR_OUT_OF_MEMORY";
        case KEYLT_ERR_BUFFER_TOO_SMALL:
            return "KEYLT_ERR_BUFFER_TOO_SMALL";
        case KEYLT_ERR_INTERNAL:
            return "KEYLT_ERR_INTERNAL";
        default:
            return "KEYLT_ERR_UNKNOWN";
    }
}

// --------------------------------------------------------------------------
// Device enumeration
// --------------------------------------------------------------------------

KEYLT_API keylt_status KEYLT_CALL keylt_list_known_keyboards(
    keylt_keyboard_info* out_infos, size_t capacity, size_t* out_count) {
    return Guard([&] { return CopyInfos(keylt::known_keyboards, out_infos, capacity, out_count); });
}

KEYLT_API keylt_status KEYLT_CALL keylt_list_available_keyboards(
    keylt_keyboard_info* out_infos, size_t capacity, size_t* out_count) {
    return Guard([&] {
        const std::vector<keylt::KeyboardInfo> attached = keylt::ListAvailableKeyboards();
        return CopyInfos(attached, out_infos, capacity, out_count);
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_model_get_info(
    keylt_model model, keylt_keyboard_info* out_info) {
    return Guard([&]() -> keylt_status {
        if (out_info == nullptr) {
            throw std::invalid_argument("out_info must not be null");
        }
        RequireImplementedModel(model);
        const keylt::KeyboardModel wanted = ToKeyboardModel(model);
        for (const auto& info : keylt::known_keyboards) {
            if (info.keyboard_model == wanted) {
                *out_info = ToCInfo(info);
                return KEYLT_OK;
            }
        }
        throw std::invalid_argument("no descriptor registered for the requested model");
    });
}

// --------------------------------------------------------------------------
// Key name introspection
// --------------------------------------------------------------------------

KEYLT_API keylt_status KEYLT_CALL keylt_model_key_count(
    keylt_model model, size_t* out_count) {
    return Guard([&]() -> keylt_status {
        if (out_count == nullptr) {
            throw std::invalid_argument("out_count must not be null");
        }
        *out_count = KeyTableFor(model).size();
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_model_key_at(
    keylt_model model, size_t index, const char** out_name, uint8_t* out_key_id) {
    return Guard([&]() -> keylt_status {
        const KeyTable& table = KeyTableFor(model);
        if (index >= table.size()) {
            throw std::out_of_range("key index " + std::to_string(index) +
                                    " is beyond the table size " +
                                    std::to_string(table.size()));
        }
        if (out_name != nullptr) {
            *out_name = table[index].first;
        }
        if (out_key_id != nullptr) {
            *out_key_id = table[index].second;
        }
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_model_key_id_for_name(
    keylt_model model, const char* name, uint8_t* out_key_id) {
    return Guard([&]() -> keylt_status {
        if (name == nullptr) {
            throw std::invalid_argument("name must not be null");
        }
        if (out_key_id == nullptr) {
            throw std::invalid_argument("out_key_id must not be null");
        }
        for (const auto& entry : KeyTableFor(model)) {
            if (std::strcmp(entry.first, name) == 0) {
                *out_key_id = entry.second;
                return KEYLT_OK;
            }
        }
        throw std::invalid_argument(std::string("unknown key name: ") + name);
    });
}

// --------------------------------------------------------------------------
// Keyboard LED control
// --------------------------------------------------------------------------

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_open(
    keylt_model model, keylt_keyboard** out_keyboard) {
    return Guard([&]() -> keylt_status {
        if (out_keyboard == nullptr) {
            throw std::invalid_argument("out_keyboard must not be null");
        }
        *out_keyboard = nullptr;
        RequireImplementedModel(model);
        *out_keyboard = new keylt_keyboard(model, ToKeyboardModel(model));
        return KEYLT_OK;
    });
}

KEYLT_API void KEYLT_CALL keylt_keyboard_close(keylt_keyboard* keyboard) {
    // The sole destroy path. keylt::Keyboard's destructor releases the device
    // handle and the model driver, so nothing else needs calling first.
    delete keyboard;
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_connect(keylt_keyboard* keyboard) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        if (!keyboard->keyboard.ConnectToDevice()) {
            return Fail(KEYLT_ERR_DEVICE_NOT_FOUND, "no matching keyboard is attached");
        }
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_is_connected(
    const keylt_keyboard* keyboard, int32_t* out_connected) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        if (out_connected == nullptr) {
            throw std::invalid_argument("out_connected must not be null");
        }
        *out_connected = const_cast<keylt::Keyboard&>(keyboard->keyboard).Found() ? 1 : 0;
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_get_info(
    const keylt_keyboard* keyboard, keylt_keyboard_info* out_info) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        return keylt_model_get_info(keyboard->model, out_info);
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_active_key_ids(
    keylt_keyboard* keyboard, const uint8_t* key_ids, size_t count) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        if (key_ids == nullptr && count > 0) {
            throw std::invalid_argument("key_ids must not be null when count is non-zero");
        }

        const keylt::AbstractKeyboard* spec = keyboard->keyboard.keyboard_spec;
        // Validate the whole batch before touching the keyboard, so a rejected
        // call leaves the previous selection intact.
        for (size_t i = 0; i < count; ++i) {
            if (key_ids[i] > spec->max_key_id) {
                throw std::out_of_range(
                    "key id " + std::to_string(static_cast<unsigned>(key_ids[i])) +
                    " exceeds the model maximum of " +
                    std::to_string(static_cast<unsigned>(spec->max_key_id)));
            }
        }
        if (count >= static_cast<size_t>(keylt::Keyboard::kMaxActiveKeys)) {
            throw std::out_of_range("too many active keys: " + std::to_string(count));
        }

        // The library's buffer is char; the ABI uses uint8_t so that key ids do
        // not depend on whether plain char is signed on this target.
        std::vector<char> as_chars(count);
        for (size_t i = 0; i < count; ++i) {
            as_chars[i] = static_cast<char>(key_ids[i]);
        }
        keyboard->keyboard.SetActiveKeyIds(as_chars.data(), static_cast<UINT8>(count));
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_active_key_names(
    keylt_keyboard* keyboard, const char* const* names, size_t count) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        if (names == nullptr && count > 0) {
            throw std::invalid_argument("names must not be null when count is non-zero");
        }

        // Resolve every name up front so an unknown one aborts the call without
        // having modified the selection.
        const KeyTable& table = KeyTableFor(keyboard->model);
        std::vector<uint8_t> ids(count);
        for (size_t i = 0; i < count; ++i) {
            if (names[i] == nullptr) {
                throw std::invalid_argument("names[" + std::to_string(i) + "] is null");
            }
            bool found = false;
            for (const auto& entry : table) {
                if (std::strcmp(entry.first, names[i]) == 0) {
                    ids[i] = entry.second;
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::invalid_argument(std::string("unknown key name: ") + names[i]);
            }
        }
        return keylt_keyboard_set_active_key_ids(keyboard, ids.data(), count);
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_get_active_key_count(
    const keylt_keyboard* keyboard, size_t* out_count) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        if (out_count == nullptr) {
            throw std::invalid_argument("out_count must not be null");
        }
        *out_count = keyboard->keyboard.n_active_keys;
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_keys(
    keylt_keyboard* keyboard, keylt_key_value value) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        const keylt::KeyValue key_value = ToKeyValue(value);
        if (!keyboard->keyboard.Found()) {
            return Fail(KEYLT_ERR_DEVICE_NOT_FOUND, "keyboard is not connected");
        }
        keyboard->keyboard.SetKeysOnOff(key_value);
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_keys_capture(
    keylt_keyboard* keyboard, keylt_key_value value,
    uint8_t* out_messages, size_t capacity, size_t* out_written) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        if (out_messages == nullptr) {
            throw std::invalid_argument("out_messages must not be null");
        }
        const keylt::KeyValue key_value = ToKeyValue(value);

        const keylt::AbstractKeyboard* spec = keyboard->keyboard.keyboard_spec;
        const size_t required = static_cast<size_t>(spec->BULK_LED_VALUE_MESSAGES_COUNT) *
                                static_cast<size_t>(spec->MESSAGE_LENGTH);
        if (out_written != nullptr) {
            *out_written = required;
        }
        // Checked before sending: the library writes the full packet set
        // unconditionally, so a short buffer would be a heap overflow.
        if (capacity < required) {
            return Fail(KEYLT_ERR_BUFFER_TOO_SMALL,
                        ("out_messages needs " + std::to_string(required) +
                         " bytes, got " + std::to_string(capacity)));
        }
        if (!keyboard->keyboard.Found()) {
            return Fail(KEYLT_ERR_DEVICE_NOT_FOUND, "keyboard is not connected");
        }

        keyboard->keyboard.SetKeysOnOff(key_value, out_messages);
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_keys_rgb(
    keylt_keyboard* keyboard, uint8_t r, uint8_t g, uint8_t b) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        if (!keyboard->keyboard.Found()) {
            return Fail(KEYLT_ERR_DEVICE_NOT_FOUND, "keyboard is not connected");
        }
        keyboard->keyboard.SetKeysRGB(r, g, b);
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_blink(
    keylt_keyboard* keyboard, int32_t count, int32_t interval_ms) {
    return Guard([&]() -> keylt_status {
        if (keyboard == nullptr) {
            throw std::invalid_argument("keyboard must not be null");
        }
        if (count < 0 || interval_ms < 0) {
            throw std::invalid_argument("count and interval_ms must not be negative");
        }
        if (!keyboard->keyboard.Found()) {
            return Fail(KEYLT_ERR_DEVICE_NOT_FOUND, "keyboard is not connected");
        }
        keyboard->keyboard.BlinkActiveKeys(static_cast<int>(count),
                                           static_cast<int>(interval_ms));
        return KEYLT_OK;
    });
}

// --------------------------------------------------------------------------
// LCD control
// --------------------------------------------------------------------------

KEYLT_API keylt_status KEYLT_CALL keylt_sk80_lcd_defaults(
    uint16_t* out_vid, uint16_t* out_pid,
    const char** out_control_interface, const char** out_data_interface) {
    return Guard([&]() -> keylt_status {
        if (out_vid != nullptr) {
            *out_vid = static_cast<uint16_t>(sk80::VID);
        }
        if (out_pid != nullptr) {
            *out_pid = static_cast<uint16_t>(sk80::PID);
        }
        if (out_control_interface != nullptr) {
            *out_control_interface = sk80::target_device_path;
        }
        if (out_data_interface != nullptr) {
            *out_data_interface = sk80::lcd_data_device_path;
        }
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_lcd_open(
    const keylt_usb_io* io, uint16_t vid, uint16_t pid,
    const char* control_interface, const char* data_interface,
    keylt_lcd** out_lcd) {
    return Guard([&]() -> keylt_status {
        if (out_lcd == nullptr) {
            throw std::invalid_argument("out_lcd must not be null");
        }
        *out_lcd = nullptr;
        if (control_interface == nullptr || data_interface == nullptr) {
            throw std::invalid_argument("interface selectors must not be null");
        }

        std::unique_ptr<keylt::IUsbIO> transport;
        if (io == nullptr) {
            transport.reset(new keylt::RealUsbIO());
        } else {
            transport.reset(new CApiUsbIO(*io));
        }

        *out_lcd = new keylt_lcd(std::move(transport), vid, pid,
                                 std::string(control_interface),
                                 std::string(data_interface));
        return KEYLT_OK;
    });
}

KEYLT_API void KEYLT_CALL keylt_lcd_close(keylt_lcd* lcd) {
    delete lcd;
}

KEYLT_API keylt_status KEYLT_CALL keylt_lcd_connect(keylt_lcd* lcd) {
    return Guard([&]() -> keylt_status {
        if (lcd == nullptr) {
            throw std::invalid_argument("lcd must not be null");
        }
        if (!lcd->lcd.ConnectToDevice()) {
            return Fail(KEYLT_ERR_DEVICE_NOT_FOUND, "no matching LCD device is attached");
        }
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_lcd_is_connected(
    const keylt_lcd* lcd, int32_t* out_connected) {
    return Guard([&]() -> keylt_status {
        if (lcd == nullptr) {
            throw std::invalid_argument("lcd must not be null");
        }
        if (out_connected == nullptr) {
            throw std::invalid_argument("out_connected must not be null");
        }
        *out_connected = const_cast<keylt::KeyboardLcd&>(lcd->lcd).Found() ? 1 : 0;
        return KEYLT_OK;
    });
}

KEYLT_API keylt_status KEYLT_CALL keylt_lcd_upload_image(
    keylt_lcd* lcd, const char* image_path) {
    return Guard([&]() -> keylt_status {
        if (lcd == nullptr) {
            throw std::invalid_argument("lcd must not be null");
        }
        if (image_path == nullptr) {
            throw std::invalid_argument("image_path must not be null");
        }
        if (!lcd->lcd.UploadImage(std::string(image_path))) {
            return Fail(KEYLT_ERR_IO, "the image upload did not complete");
        }
        return KEYLT_OK;
    });
}

// --------------------------------------------------------------------------
// Diagnostics
// --------------------------------------------------------------------------

KEYLT_API keylt_status KEYLT_CALL keylt_print_messages(
    const uint8_t* buffer, size_t message_count, size_t message_length) {
    return Guard([&]() -> keylt_status {
        if (buffer == nullptr && message_count != 0 && message_length != 0) {
            throw std::invalid_argument("buffer must not be null");
        }
        keylt::PrintMessagesInBuffer(buffer, message_count, message_length);
        return KEYLT_OK;
    });
}

}  // extern "C"
