//! FFI declarations for the `keylt` C ABI, transcribed from `include/main/keylt.h`.
//!
//! This file is the default binding source. It is maintained by hand so that
//! building this crate needs no libclang; enable the `bindgen` feature to
//! generate the equivalent declarations from the header instead. The two are
//! kept honest by the layout and signature assertions in `tests/`, which run
//! against whichever source is active.
//!
//! Shape notes, chosen to match `bindgen --default-enum-style=consts`:
//!
//! * Enums become a type alias plus a set of constants rather than a Rust
//!   `enum`. A `#[repr(C)] enum` holding a discriminant the Rust definition does
//!   not list is undefined behaviour, and the library is free to add status
//!   codes in a future version.
//! * `keylt_status` has negative enumerators, so C gives it a signed underlying
//!   type; `keylt_model` and `keylt_key_value` do not, so theirs is unsigned.

use std::os::raw::{c_char, c_int, c_uint, c_void};

pub const KEYLT_DISPLAY_NAME_CAPACITY: u32 = 25;

pub const KEYLT_ABI_VERSION_MAJOR: u32 = 1;
pub const KEYLT_ABI_VERSION_MINOR: u32 = 0;
pub const KEYLT_ABI_VERSION_PATCH: u32 = 0;

/* ----------------------------------------------------------------------- */
/* Status codes                                                            */
/* ----------------------------------------------------------------------- */

pub type keylt_status = c_int;

pub const KEYLT_OK: keylt_status = 0;
pub const KEYLT_ERR_INVALID_ARGUMENT: keylt_status = -1;
pub const KEYLT_ERR_DEVICE_NOT_FOUND: keylt_status = -2;
pub const KEYLT_ERR_OUT_OF_RANGE: keylt_status = -3;
pub const KEYLT_ERR_UNSUPPORTED: keylt_status = -4;
pub const KEYLT_ERR_IO: keylt_status = -5;
pub const KEYLT_ERR_OUT_OF_MEMORY: keylt_status = -6;
pub const KEYLT_ERR_BUFFER_TOO_SMALL: keylt_status = -7;
pub const KEYLT_ERR_INTERNAL: keylt_status = -99;

/* ----------------------------------------------------------------------- */
/* Enumerations                                                            */
/* ----------------------------------------------------------------------- */

pub type keylt_model = c_uint;

pub const KEYLT_MODEL_SK80: keylt_model = 0;
pub const KEYLT_MODEL_MK84: keylt_model = 1;
pub const KEYLT_MODEL_RK84: keylt_model = 2;

pub type keylt_key_value = c_uint;

pub const KEYLT_KEY_OFF: keylt_key_value = 0;
pub const KEYLT_KEY_ON: keylt_key_value = 1;

/* ----------------------------------------------------------------------- */
/* Plain data                                                              */
/* ----------------------------------------------------------------------- */

/// Describes one keyboard model. Fixed-width fields only, so the layout is
/// reproducible in any language that can express a C struct.
#[repr(C)]
#[derive(Debug, Default, Copy, Clone, PartialEq, Eq, Hash)]
pub struct keylt_keyboard_info {
    /// A `keylt_model` value.
    pub model: i32,
    pub vid: u16,
    pub pid: u16,
    pub max_key_id: u8,
    pub message_length: u8,
    pub bulk_led_value_messages_count: u8,
    /// Always 0; keeps the layout explicit.
    pub reserved: u8,
    /// NUL-terminated and always fully initialised.
    pub display_name: [c_char; 25usize],
    /// Always 0.
    pub padding: [c_char; 3usize],
}

/* ----------------------------------------------------------------------- */
/* Opaque handles                                                          */
/* ----------------------------------------------------------------------- */

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct keylt_keyboard {
    _unused: [u8; 0],
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct keylt_lcd {
    _unused: [u8; 0],
}

/* ----------------------------------------------------------------------- */
/* Injectable USB transport                                                */
/* ----------------------------------------------------------------------- */

/// A caller-supplied USB backend. Every callback receives the `user_data`
/// pointer from the containing struct as its first argument.
///
/// A NULL callback is treated as an unimplemented operation and makes the
/// corresponding library call fail with `KEYLT_ERR_UNSUPPORTED`. Callbacks must
/// not unwind.
///
/// `Default` is implemented in `lib.rs` rather than derived: the `user_data`
/// raw pointer has no `Default`, which is also why bindgen omits it.
///
/// Equality and hashing are deliberately not derived either. Comparing function
/// pointers is meaningless -- identical functions may be merged, and the same
/// function may have different addresses in different codegen units -- and
/// rustc warns about it.
#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct keylt_usb_io {
    pub user_data: *mut c_void,
    pub open: Option<
        unsafe extern "C" fn(
            user_data: *mut c_void,
            vid: u16,
            pid: u16,
            control_interface: *const c_char,
            data_interface: *const c_char,
        ) -> i32,
    >,
    pub send_feature_report:
        Option<unsafe extern "C" fn(user_data: *mut c_void, data: *const u8, length: usize) -> i32>,
    pub get_feature_report:
        Option<unsafe extern "C" fn(user_data: *mut c_void, buffer: *mut u8, length: usize) -> i32>,
    pub write_data:
        Option<unsafe extern "C" fn(user_data: *mut c_void, data: *const u8, length: usize) -> i32>,
    pub read_data: Option<
        unsafe extern "C" fn(
            user_data: *mut c_void,
            buffer: *mut u8,
            length: usize,
            timeout_ms: i32,
        ) -> i32,
    >,
    pub close: Option<unsafe extern "C" fn(user_data: *mut c_void)>,
}

extern "C" {
    /* ------------------------------------------------------------------- */
    /* Library information                                                 */
    /* ------------------------------------------------------------------- */

    pub fn keylt_abi_version() -> u32;

    pub fn keylt_version_string() -> *const c_char;

    /* ------------------------------------------------------------------- */
    /* Errors                                                              */
    /* ------------------------------------------------------------------- */

    pub fn keylt_last_error_message() -> *const c_char;

    pub fn keylt_clear_last_error();

    pub fn keylt_status_name(status: i32) -> *const c_char;

    /* ------------------------------------------------------------------- */
    /* Device enumeration                                                  */
    /* ------------------------------------------------------------------- */

    pub fn keylt_list_known_keyboards(
        out_infos: *mut keylt_keyboard_info,
        capacity: usize,
        out_count: *mut usize,
    ) -> keylt_status;

    pub fn keylt_list_available_keyboards(
        out_infos: *mut keylt_keyboard_info,
        capacity: usize,
        out_count: *mut usize,
    ) -> keylt_status;

    pub fn keylt_model_get_info(
        model: keylt_model,
        out_info: *mut keylt_keyboard_info,
    ) -> keylt_status;

    /* ------------------------------------------------------------------- */
    /* Key name introspection                                              */
    /* ------------------------------------------------------------------- */

    pub fn keylt_model_key_count(model: keylt_model, out_count: *mut usize) -> keylt_status;

    pub fn keylt_model_key_at(
        model: keylt_model,
        index: usize,
        out_name: *mut *const c_char,
        out_key_id: *mut u8,
    ) -> keylt_status;

    pub fn keylt_model_key_id_for_name(
        model: keylt_model,
        name: *const c_char,
        out_key_id: *mut u8,
    ) -> keylt_status;

    /* ------------------------------------------------------------------- */
    /* Keyboard LED control                                                */
    /* ------------------------------------------------------------------- */

    pub fn keylt_keyboard_open(
        model: keylt_model,
        out_keyboard: *mut *mut keylt_keyboard,
    ) -> keylt_status;

    pub fn keylt_keyboard_close(keyboard: *mut keylt_keyboard);

    pub fn keylt_keyboard_connect(keyboard: *mut keylt_keyboard) -> keylt_status;

    pub fn keylt_keyboard_is_connected(
        keyboard: *const keylt_keyboard,
        out_connected: *mut i32,
    ) -> keylt_status;

    pub fn keylt_keyboard_get_info(
        keyboard: *const keylt_keyboard,
        out_info: *mut keylt_keyboard_info,
    ) -> keylt_status;

    pub fn keylt_keyboard_set_active_key_ids(
        keyboard: *mut keylt_keyboard,
        key_ids: *const u8,
        count: usize,
    ) -> keylt_status;

    pub fn keylt_keyboard_set_active_key_names(
        keyboard: *mut keylt_keyboard,
        names: *const *const c_char,
        count: usize,
    ) -> keylt_status;

    pub fn keylt_keyboard_get_active_key_count(
        keyboard: *const keylt_keyboard,
        out_count: *mut usize,
    ) -> keylt_status;

    pub fn keylt_keyboard_set_keys(
        keyboard: *mut keylt_keyboard,
        value: keylt_key_value,
    ) -> keylt_status;

    pub fn keylt_keyboard_set_keys_capture(
        keyboard: *mut keylt_keyboard,
        value: keylt_key_value,
        out_messages: *mut u8,
        capacity: usize,
        out_written: *mut usize,
    ) -> keylt_status;

    pub fn keylt_keyboard_set_keys_rgb(
        keyboard: *mut keylt_keyboard,
        r: u8,
        g: u8,
        b: u8,
    ) -> keylt_status;

    pub fn keylt_keyboard_blink(
        keyboard: *mut keylt_keyboard,
        count: i32,
        interval_ms: i32,
    ) -> keylt_status;

    /* ------------------------------------------------------------------- */
    /* LCD control                                                         */
    /* ------------------------------------------------------------------- */

    pub fn keylt_sk80_lcd_defaults(
        out_vid: *mut u16,
        out_pid: *mut u16,
        out_control_interface: *mut *const c_char,
        out_data_interface: *mut *const c_char,
    ) -> keylt_status;

    pub fn keylt_lcd_open(
        io: *const keylt_usb_io,
        vid: u16,
        pid: u16,
        control_interface: *const c_char,
        data_interface: *const c_char,
        out_lcd: *mut *mut keylt_lcd,
    ) -> keylt_status;

    pub fn keylt_lcd_close(lcd: *mut keylt_lcd);

    pub fn keylt_lcd_connect(lcd: *mut keylt_lcd) -> keylt_status;

    pub fn keylt_lcd_is_connected(lcd: *const keylt_lcd, out_connected: *mut i32) -> keylt_status;

    pub fn keylt_lcd_upload_image(lcd: *mut keylt_lcd, image_path: *const c_char) -> keylt_status;

    /* ------------------------------------------------------------------- */
    /* Diagnostics                                                         */
    /* ------------------------------------------------------------------- */

    pub fn keylt_print_messages(
        buffer: *const u8,
        message_count: usize,
        message_length: usize,
    ) -> keylt_status;
}
