#include "keylt_loader.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <cstdio>
#include <cstring>
#include <iostream>

// Only for the ABI's types and function signatures. The demo never links
// against keylt, so every entry point is reached through a resolved pointer.
#include "keylt.h"

namespace demo {
namespace {

// --------------------------------------------------------------------------
// Platform shims
// --------------------------------------------------------------------------

#ifdef _WIN32
using LibHandle = HMODULE;

LibHandle OpenSharedLibrary() {
    return LoadLibrary(TEXT("keylt.dll"));
}

void* ResolveSymbol(LibHandle handle, const char* name) {
    return reinterpret_cast<void*>(GetProcAddress(handle, name));
}

std::string OpenError() {
    return "LoadLibrary(keylt.dll) failed";
}
#else
using LibHandle = void*;

LibHandle OpenSharedLibrary() {
    // Alongside the executable first, then wherever the loader looks.
    LibHandle handle = dlopen("./libkeylt.so", RTLD_LAZY);
    if (!handle) {
        handle = dlopen("libkeylt.so", RTLD_LAZY);
    }
    return handle;
}

void* ResolveSymbol(LibHandle handle, const char* name) {
    return dlsym(handle, name);
}

std::string OpenError() {
    const char* detail = dlerror();
    return std::string("dlopen(libkeylt.so) failed: ") + (detail ? detail : "unknown");
}
#endif

// --------------------------------------------------------------------------
// Resolved entry points
// --------------------------------------------------------------------------

// Signatures mirror keylt.h exactly. They are spelled out here rather than
// taken from the header so that a future ABI change shows up as a compile
// error in one place.
struct Api {
    const char* (KEYLT_CALL* last_error_message)(void);
    keylt_status(KEYLT_CALL* list_available_keyboards)(keylt_keyboard_info*, size_t, size_t*);
    keylt_status(KEYLT_CALL* keyboard_open)(keylt_model, keylt_keyboard**);
    void (KEYLT_CALL* keyboard_close)(keylt_keyboard*);
    keylt_status(KEYLT_CALL* keyboard_connect)(keylt_keyboard*);
    keylt_status(KEYLT_CALL* keyboard_set_active_key_ids)(keylt_keyboard*, const uint8_t*, size_t);
    keylt_status(KEYLT_CALL* keyboard_set_active_key_names)(keylt_keyboard*, const char* const*, size_t);
    keylt_status(KEYLT_CALL* keyboard_set_keys)(keylt_keyboard*, keylt_key_value);
    keylt_status(KEYLT_CALL* keyboard_set_keys_capture)(keylt_keyboard*, keylt_key_value, uint8_t*, size_t, size_t*);
    keylt_status(KEYLT_CALL* print_messages)(const uint8_t*, size_t, size_t);
};

std::string& LastError() {
    static std::string message;
    return message;
}

// Resolves every symbol once, on first use. Returns nullptr if the library or
// any symbol is missing, in which case LastError() explains why.
const Api* GetApi() {
    static bool initialised = false;
    static bool usable = false;
    static Api api{};

    if (initialised) {
        return usable ? &api : nullptr;
    }
    initialised = true;

    // Never released: the pointers stay live for the life of the process, and
    // unloading underneath them would invalidate every handle.
    LibHandle handle = OpenSharedLibrary();
    if (!handle) {
        LastError() = OpenError();
        std::cerr << LastError() << std::endl;
        return nullptr;
    }

    struct Binding {
        void** slot;
        const char* name;
    };
    const Binding bindings[] = {
        {reinterpret_cast<void**>(&api.last_error_message), "keylt_last_error_message"},
        {reinterpret_cast<void**>(&api.list_available_keyboards), "keylt_list_available_keyboards"},
        {reinterpret_cast<void**>(&api.keyboard_open), "keylt_keyboard_open"},
        {reinterpret_cast<void**>(&api.keyboard_close), "keylt_keyboard_close"},
        {reinterpret_cast<void**>(&api.keyboard_connect), "keylt_keyboard_connect"},
        {reinterpret_cast<void**>(&api.keyboard_set_active_key_ids), "keylt_keyboard_set_active_key_ids"},
        {reinterpret_cast<void**>(&api.keyboard_set_active_key_names), "keylt_keyboard_set_active_key_names"},
        {reinterpret_cast<void**>(&api.keyboard_set_keys), "keylt_keyboard_set_keys"},
        {reinterpret_cast<void**>(&api.keyboard_set_keys_capture), "keylt_keyboard_set_keys_capture"},
        {reinterpret_cast<void**>(&api.print_messages), "keylt_print_messages"},
    };

    for (const Binding& binding : bindings) {
        void* symbol = ResolveSymbol(handle, binding.name);
        if (!symbol) {
            LastError() = std::string("missing symbol: ") + binding.name;
            std::cerr << LastError() << std::endl;
            return nullptr;
        }
        *binding.slot = symbol;
    }

    usable = true;
    return &api;
}

// Copies the library's error message into our own slot so callers see it even
// after the handle has been closed.
int Record(const Api& api, keylt_status status) {
    if (status == KEYLT_OK) {
        LastError().clear();
        return 0;
    }
    const char* detail = api.last_error_message();
    LastError() = detail ? detail : "";
    return 1;
}

keylt_model ToAbiModel(keylt::KeyboardModel model) {
    switch (model) {
        case keylt::kSK80:
            return KEYLT_MODEL_SK80;
        case keylt::kMK84:
            return KEYLT_MODEL_MK84;
        case keylt::kRK84:
            return KEYLT_MODEL_RK84;
    }
    return KEYLT_MODEL_SK80;
}

keylt::KeyboardModel FromAbiModel(int32_t model) {
    switch (model) {
        case KEYLT_MODEL_MK84:
            return keylt::kMK84;
        case KEYLT_MODEL_RK84:
            return keylt::kRK84;
        case KEYLT_MODEL_SK80:
        default:
            return keylt::kSK80;
    }
}

keylt::KeyboardInfo FromAbiInfo(const keylt_keyboard_info& info) {
    keylt::KeyboardInfo out{};
    out.keyboard_model = FromAbiModel(info.model);
    out.vid = static_cast<short>(info.vid);
    out.pid = static_cast<short>(info.pid);
    out.max_key_id = info.max_key_id;
    out.MESSAGE_LENGTH = info.message_length;
    out.BULK_LED_VALUE_MESSAGES_COUNT = info.bulk_led_value_messages_count;
    std::snprintf(out.display_name, sizeof(out.display_name), "%s", info.display_name);
    return out;
}

// Opens and connects a keyboard, running `body` with the live handle. The
// handle is always closed, including on the failure paths.
template <typename Body>
int WithConnectedKeyboard(keylt::KeyboardInfo keyboard, Body body) {
    const Api* api = GetApi();
    if (!api) {
        return 1;
    }

    keylt_keyboard* handle = nullptr;
    keylt_status status = api->keyboard_open(ToAbiModel(keyboard.keyboard_model), &handle);
    if (status != KEYLT_OK) {
        return Record(*api, status);
    }

    status = api->keyboard_connect(handle);
    if (status == KEYLT_OK) {
        status = body(*api, handle);
    }

    const int result = Record(*api, status);
    api->keyboard_close(handle);
    return result;
}

int SetKeyNames(const std::vector<std::string>& key_names, keylt::KeyboardInfo keyboard,
                keylt_key_value value) {
    std::vector<const char*> names;
    names.reserve(key_names.size());
    for (const std::string& name : key_names) {
        names.push_back(name.c_str());
    }

    return WithConnectedKeyboard(keyboard, [&](const Api& api, keylt_keyboard* handle) {
        keylt_status status =
            api.keyboard_set_active_key_names(handle, names.data(), names.size());
        if (status != KEYLT_OK) {
            return status;
        }
        return api.keyboard_set_keys(handle, value);
    });
}

}  // namespace

// --------------------------------------------------------------------------
// Public helpers
// --------------------------------------------------------------------------

std::string LastLoaderError() {
    return LastError();
}

int CallDllTurnOnKeyNames(const std::vector<std::string>& key_names, keylt::KeyboardInfo keyboard) {
    return SetKeyNames(key_names, keyboard, KEYLT_KEY_ON);
}

int CallDllTurnOffKeyNames(const std::vector<std::string>& key_names, keylt::KeyboardInfo keyboard) {
    return SetKeyNames(key_names, keyboard, KEYLT_KEY_OFF);
}

int CallTurnOnKeyIdsD(char* key_ids, UINT8 n_keys, unsigned char* messages_sent,
                      keylt::KeyboardInfo keyboard) {
    if (key_ids == nullptr || messages_sent == nullptr) {
        LastError() = "CallTurnOnKeyIdsD was given a null buffer";
        return 1;
    }

    std::vector<uint8_t> ids(key_ids, key_ids + n_keys);
    // The ABI requires an explicit capacity; this is what the caller sized the
    // buffer to.
    const size_t capacity = static_cast<size_t>(keyboard.BULK_LED_VALUE_MESSAGES_COUNT) *
                            static_cast<size_t>(keyboard.MESSAGE_LENGTH);

    return WithConnectedKeyboard(keyboard, [&](const Api& api, keylt_keyboard* handle) {
        keylt_status status = api.keyboard_set_active_key_ids(handle, ids.data(), ids.size());
        if (status != KEYLT_OK) {
            return status;
        }
        return api.keyboard_set_keys_capture(handle, KEYLT_KEY_ON, messages_sent, capacity,
                                             nullptr);
    });
}

void CallPrintMessagesInBuffer(unsigned char* buffer, size_t message_count, size_t message_length) {
    const Api* api = GetApi();
    if (!api) {
        return;
    }
    Record(*api, api->print_messages(buffer, message_count, message_length));
}

std::vector<keylt::KeyboardInfo> ListAvailableKeyboards() {
    std::vector<keylt::KeyboardInfo> list;

    const Api* api = GetApi();
    if (!api) {
        return list;
    }

    // Probe for the count, then fetch. The library allocates nothing on our
    // behalf, so there is no matching free to get wrong.
    size_t count = 0;
    keylt_status status = api->list_available_keyboards(nullptr, 0, &count);
    if (status != KEYLT_OK && status != KEYLT_ERR_BUFFER_TOO_SMALL) {
        Record(*api, status);
        return list;
    }
    if (count == 0) {
        return list;
    }

    std::vector<keylt_keyboard_info> infos(count);
    status = api->list_available_keyboards(infos.data(), infos.size(), &count);
    if (status != KEYLT_OK) {
        Record(*api, status);
        return list;
    }

    list.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        list.push_back(FromAbiInfo(infos[i]));
    }
    return list;
}

}  // namespace demo
