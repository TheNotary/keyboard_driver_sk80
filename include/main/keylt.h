/*
 * keylt - portable C ABI for controlling RGB mechanical keyboard LEDs and LCDs.
 *
 * This header is the complete public interface of the library. It is valid C99
 * and C++; it includes no C++ headers, declares no C++ types, and every symbol
 * it declares has C language linkage. It is intended to be consumed directly by
 * bindgen and other FFI generators.
 *
 * Conventions
 *   - Every function returns keylt_status. Results are written through
 *     out-parameters. Nothing is returned by value except the accessors that
 *     yield a `const char*` with static storage duration.
 *   - No function throws. Failures are reported as a negative keylt_status;
 *     keylt_last_error_message() supplies a human-readable description.
 *   - Object lifetimes use opaque handles created by a `*_open` function and
 *     released by the matching `*_close` function. No memory is ever allocated
 *     by the library for the caller to free: buffers are always supplied by the
 *     caller with an explicit capacity.
 *   - The library performs no locking and creates no threads. A handle must not
 *     be used from two threads at once. Distinct handles may be used from
 *     distinct threads; the last-error slot is thread-local.
 */
#ifndef KEYLT_H
#define KEYLT_H

#include <stddef.h>
#include <stdint.h>

/*
 * KEYLT_STATIC is set by the keylt::keylt_static target's usage requirements;
 * consumers of the static archive must not see __declspec(dllimport).
 */
#if defined(_WIN32) && !defined(KEYLT_STATIC)
#ifdef BUILD_DLL
#define KEYLT_API __declspec(dllexport)
#else
#define KEYLT_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
/* The library is built with -fvisibility=hidden, so the public API has to opt
 * back in explicitly. */
#define KEYLT_API __attribute__((visibility("default")))
#else
#define KEYLT_API
#endif

/* Spelled out so foreign callers do not have to guess the convention. */
#if defined(_WIN32)
#define KEYLT_CALL __cdecl
#else
#define KEYLT_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* Status codes                                                              */
/* ------------------------------------------------------------------------- */

/* Success is 0; every failure is negative, so `status < 0` is a valid test. */
typedef enum keylt_status {
    KEYLT_OK = 0,
    KEYLT_ERR_INVALID_ARGUMENT = -1,
    KEYLT_ERR_DEVICE_NOT_FOUND = -2,
    KEYLT_ERR_OUT_OF_RANGE = -3,
    KEYLT_ERR_UNSUPPORTED = -4,
    KEYLT_ERR_IO = -5,
    KEYLT_ERR_OUT_OF_MEMORY = -6,
    KEYLT_ERR_BUFFER_TOO_SMALL = -7,
    KEYLT_ERR_INTERNAL = -99
} keylt_status;

/* ------------------------------------------------------------------------- */
/* Enumerations                                                              */
/* ------------------------------------------------------------------------- */

typedef enum keylt_model {
    KEYLT_MODEL_SK80 = 0,
    KEYLT_MODEL_MK84 = 1,
    KEYLT_MODEL_RK84 = 2
} keylt_model;

/* Note the ordering: OFF is 0 and ON is 1, matching the intuitive reading
 * rather than the library's internal enum. Never cast between the two. */
typedef enum keylt_key_value {
    KEYLT_KEY_OFF = 0,
    KEYLT_KEY_ON = 1
} keylt_key_value;

/* ------------------------------------------------------------------------- */
/* Plain data                                                                */
/* ------------------------------------------------------------------------- */

#define KEYLT_DISPLAY_NAME_CAPACITY 25

/*
 * Describes one keyboard model. Fixed-width fields only, so the layout is
 * reproducible in any language that can express a C struct.
 *
 * display_name is NUL-terminated and always fully initialised (trailing bytes
 * are zeroed), so it is safe to read as a C string.
 */
typedef struct keylt_keyboard_info {
    int32_t model;                                 /* a keylt_model value */
    uint16_t vid;
    uint16_t pid;
    uint8_t max_key_id;
    uint8_t message_length;
    uint8_t bulk_led_value_messages_count;
    uint8_t reserved;                              /* always 0; keeps the layout explicit */
    char display_name[KEYLT_DISPLAY_NAME_CAPACITY];
    char padding[3];                               /* always 0 */
} keylt_keyboard_info;

/* ------------------------------------------------------------------------- */
/* Opaque handles                                                            */
/* ------------------------------------------------------------------------- */

typedef struct keylt_keyboard keylt_keyboard;
typedef struct keylt_lcd keylt_lcd;

/* ------------------------------------------------------------------------- */
/* Injectable USB transport                                                  */
/* ------------------------------------------------------------------------- */

/*
 * A caller-supplied USB backend. Every callback receives the `user_data`
 * pointer from the containing struct as its first argument.
 *
 * The read/write callbacks return the number of bytes transferred, or a
 * negative value on failure. `open` returns non-zero on success. A NULL
 * callback is treated as an unimplemented operation and makes the corresponding
 * library call fail with KEYLT_ERR_UNSUPPORTED.
 *
 * Callbacks must not unwind. The library copies the struct when the handle is
 * created, so the struct itself need not outlive the call; whatever `user_data`
 * points at must outlive the handle.
 */
typedef struct keylt_usb_io {
    void* user_data;
    int32_t(KEYLT_CALL* open)(void* user_data, uint16_t vid, uint16_t pid,
                              const char* control_interface, const char* data_interface);
    int32_t(KEYLT_CALL* send_feature_report)(void* user_data, const uint8_t* data, size_t length);
    int32_t(KEYLT_CALL* get_feature_report)(void* user_data, uint8_t* buffer, size_t length);
    int32_t(KEYLT_CALL* write_data)(void* user_data, const uint8_t* data, size_t length);
    int32_t(KEYLT_CALL* read_data)(void* user_data, uint8_t* buffer, size_t length, int32_t timeout_ms);
    void (KEYLT_CALL* close)(void* user_data);
} keylt_usb_io;

/* ------------------------------------------------------------------------- */
/* Library information                                                       */
/* ------------------------------------------------------------------------- */

/* Encoded as (major << 16) | (minor << 8) | patch. Compare against
 * KEYLT_ABI_VERSION at run time to detect a mismatched shared library. */
#define KEYLT_ABI_VERSION_MAJOR 1
#define KEYLT_ABI_VERSION_MINOR 0
#define KEYLT_ABI_VERSION_PATCH 0
#define KEYLT_ABI_VERSION                                    \
    (((uint32_t)KEYLT_ABI_VERSION_MAJOR << 16) |             \
     ((uint32_t)KEYLT_ABI_VERSION_MINOR << 8) |              \
     ((uint32_t)KEYLT_ABI_VERSION_PATCH))

KEYLT_API uint32_t KEYLT_CALL keylt_abi_version(void);

/* Human-readable library version, e.g. "0.2.0". Static storage; never NULL. */
KEYLT_API const char* KEYLT_CALL keylt_version_string(void);

/* ------------------------------------------------------------------------- */
/* Errors                                                                    */
/* ------------------------------------------------------------------------- */

/*
 * Describes the most recent failure on the calling thread. Never NULL; returns
 * an empty string when no failure has been recorded.
 *
 * The pointer is valid until the next keylt_* call on the same thread. Copy the
 * string if it needs to outlive that.
 */
KEYLT_API const char* KEYLT_CALL keylt_last_error_message(void);

KEYLT_API void KEYLT_CALL keylt_clear_last_error(void);

/* Static, human-readable name for a status code, e.g. "KEYLT_ERR_IO". Never
 * NULL; unrecognised values yield "KEYLT_ERR_UNKNOWN". */
KEYLT_API const char* KEYLT_CALL keylt_status_name(int32_t status);

/* ------------------------------------------------------------------------- */
/* Device enumeration                                                        */
/* ------------------------------------------------------------------------- */

/*
 * Writes the models keylt knows how to drive into `out_infos`.
 *
 * `out_count` always receives the total number of known models, even when the
 * buffer is too small, so the usual pattern is to call once with
 * (NULL, 0, &n) and again with a buffer of n entries.
 *
 * Returns KEYLT_ERR_BUFFER_TOO_SMALL if capacity is less than the total.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_list_known_keyboards(
    keylt_keyboard_info* out_infos, size_t capacity, size_t* out_count);

/*
 * As above, but restricted to keyboards currently attached to the system. This
 * enumerates USB devices and may block briefly.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_list_available_keyboards(
    keylt_keyboard_info* out_infos, size_t capacity, size_t* out_count);

/* Fills `out_info` for a model without opening a device. */
KEYLT_API keylt_status KEYLT_CALL keylt_model_get_info(
    keylt_model model, keylt_keyboard_info* out_info);

/* ------------------------------------------------------------------------- */
/* Key name introspection                                                    */
/* ------------------------------------------------------------------------- */

/*
 * The set of key names a model understands, exposed as a stable, sorted,
 * zero-based index range. Indices are stable for the lifetime of the process.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_model_key_count(
    keylt_model model, size_t* out_count);

/*
 * Looks up the key at `index`. `out_name` receives a pointer with static
 * storage duration -- it stays valid for the lifetime of the process and must
 * not be freed. Either out-parameter may be NULL if not wanted.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_model_key_at(
    keylt_model model, size_t index, const char** out_name, uint8_t* out_key_id);

/* Resolves a key name to its hardware key id. Returns
 * KEYLT_ERR_INVALID_ARGUMENT if the model does not define that name. */
KEYLT_API keylt_status KEYLT_CALL keylt_model_key_id_for_name(
    keylt_model model, const char* name, uint8_t* out_key_id);

/* ------------------------------------------------------------------------- */
/* Keyboard LED control                                                      */
/* ------------------------------------------------------------------------- */

/*
 * Creates a handle for `model`. This does not touch hardware; call
 * keylt_keyboard_connect() before any operation that talks to the device.
 *
 * The handle must be released with keylt_keyboard_close(), which is the only
 * destroy path and is safe to call with NULL.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_open(
    keylt_model model, keylt_keyboard** out_keyboard);

KEYLT_API void KEYLT_CALL keylt_keyboard_close(keylt_keyboard* keyboard);

/* Opens the underlying USB device. Returns KEYLT_ERR_DEVICE_NOT_FOUND when no
 * matching keyboard is attached. Safe to call more than once. */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_connect(keylt_keyboard* keyboard);

/* `out_connected` receives 1 when the device handle is open, otherwise 0. */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_is_connected(
    const keylt_keyboard* keyboard, int32_t* out_connected);

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_get_info(
    const keylt_keyboard* keyboard, keylt_keyboard_info* out_info);

/*
 * Selects the keys that subsequent keylt_keyboard_set_* calls act on.
 *
 * `key_ids` holds `count` hardware key ids; ids above the model's max_key_id
 * are rejected with KEYLT_ERR_OUT_OF_RANGE. Passing count == 0 clears the
 * selection.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_active_key_ids(
    keylt_keyboard* keyboard, const uint8_t* key_ids, size_t count);

/*
 * As above, but resolves names through the model's key table. `names` is an
 * array of `count` NUL-terminated strings. An unknown name fails the whole call
 * with KEYLT_ERR_INVALID_ARGUMENT and leaves the selection unchanged.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_active_key_names(
    keylt_keyboard* keyboard, const char* const* names, size_t count);

KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_get_active_key_count(
    const keylt_keyboard* keyboard, size_t* out_count);

/* Drives the active keys to their on or off state. */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_keys(
    keylt_keyboard* keyboard, keylt_key_value value);

/*
 * As keylt_keyboard_set_keys(), but also copies the packets that were sent into
 * `out_messages`.
 *
 * The required capacity is message_length * bulk_led_value_messages_count for
 * the model, both of which are available from keylt_keyboard_get_info(). If
 * `capacity` is smaller, nothing is sent and KEYLT_ERR_BUFFER_TOO_SMALL is
 * returned with `out_written` set to the required size. `out_written` may be
 * NULL.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_keys_capture(
    keylt_keyboard* keyboard, keylt_key_value value,
    uint8_t* out_messages, size_t capacity, size_t* out_written);

/* Sets the active keys to an RGB colour. Not every model implements this;
 * unimplemented models return KEYLT_ERR_UNSUPPORTED. */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_set_keys_rgb(
    keylt_keyboard* keyboard, uint8_t r, uint8_t g, uint8_t b);

/*
 * Cycles the active keys through a fixed colour sequence `count` times.
 *
 * This blocks the calling thread for roughly count * interval_ms * 7
 * milliseconds and cannot be cancelled. Call it from a thread that can afford
 * to stall.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_keyboard_blink(
    keylt_keyboard* keyboard, int32_t count, int32_t interval_ms);

/* ------------------------------------------------------------------------- */
/* LCD control                                                               */
/* ------------------------------------------------------------------------- */

/*
 * Reports the USB identifiers and HID interface selectors for the SK80's LCD,
 * so callers do not need to hard-code them. The strings have static storage
 * duration. Any out-parameter may be NULL.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_sk80_lcd_defaults(
    uint16_t* out_vid, uint16_t* out_pid,
    const char** out_control_interface, const char** out_data_interface);

/*
 * Creates an LCD handle.
 *
 * Pass NULL for `io` to use the library's built-in USB backend. Otherwise the
 * vtable is copied into the handle and used for all transport; this is the seam
 * for supplying a mock or an alternative HID stack.
 *
 * `control_interface` and `data_interface` are borrowed only for the duration
 * of this call.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_lcd_open(
    const keylt_usb_io* io, uint16_t vid, uint16_t pid,
    const char* control_interface, const char* data_interface,
    keylt_lcd** out_lcd);

KEYLT_API void KEYLT_CALL keylt_lcd_close(keylt_lcd* lcd);

KEYLT_API keylt_status KEYLT_CALL keylt_lcd_connect(keylt_lcd* lcd);

KEYLT_API keylt_status KEYLT_CALL keylt_lcd_is_connected(
    const keylt_lcd* lcd, int32_t* out_connected);

/*
 * Uploads an image or animated GIF at `image_path` to the display.
 *
 * This blocks for a long time -- the protocol requires a pause between each 4
 * KiB page -- and reports no progress. There is no way to cancel it.
 */
KEYLT_API keylt_status KEYLT_CALL keylt_lcd_upload_image(
    keylt_lcd* lcd, const char* image_path);

/* ------------------------------------------------------------------------- */
/* Diagnostics                                                               */
/* ------------------------------------------------------------------------- */

/* Prints `message_count` packets of `message_length` bytes to stdout as hex.
 * Intended for debugging protocol captures. */
KEYLT_API keylt_status KEYLT_CALL keylt_print_messages(
    const uint8_t* buffer, size_t message_count, size_t message_length);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* KEYLT_H */
