//! FFI declarations for the `blink` C ABI, transcribed from `include/main/blink.h`.
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
//! * `blink_status` has negative enumerators, so C gives it a signed underlying
//!   type; `blink_model` and `blink_key_value` do not, so theirs is unsigned.

use std::os::raw::{c_char, c_int, c_uint, c_void};

pub const BLINK_DISPLAY_NAME_CAPACITY: u32 = 25;

pub const BLINK_ABI_VERSION_MAJOR: u32 = 1;
pub const BLINK_ABI_VERSION_MINOR: u32 = 0;
pub const BLINK_ABI_VERSION_PATCH: u32 = 0;

/* ----------------------------------------------------------------------- */
/* Status codes                                                            */
/* ----------------------------------------------------------------------- */

pub type blink_status = c_int;

pub const BLINK_OK: blink_status = 0;
pub const BLINK_ERR_INVALID_ARGUMENT: blink_status = -1;
pub const BLINK_ERR_DEVICE_NOT_FOUND: blink_status = -2;
pub const BLINK_ERR_OUT_OF_RANGE: blink_status = -3;
pub const BLINK_ERR_UNSUPPORTED: blink_status = -4;
pub const BLINK_ERR_IO: blink_status = -5;
pub const BLINK_ERR_OUT_OF_MEMORY: blink_status = -6;
pub const BLINK_ERR_BUFFER_TOO_SMALL: blink_status = -7;
pub const BLINK_ERR_INTERNAL: blink_status = -99;

/* ----------------------------------------------------------------------- */
/* Enumerations                                                            */
/* ----------------------------------------------------------------------- */

pub type blink_model = c_uint;

pub const BLINK_MODEL_SK80: blink_model = 0;
pub const BLINK_MODEL_MK84: blink_model = 1;
pub const BLINK_MODEL_RK84: blink_model = 2;

pub type blink_key_value = c_uint;

pub const BLINK_KEY_OFF: blink_key_value = 0;
pub const BLINK_KEY_ON: blink_key_value = 1;

/* ----------------------------------------------------------------------- */
/* Plain data                                                              */
/* ----------------------------------------------------------------------- */

/// Describes one keyboard model. Fixed-width fields only, so the layout is
/// reproducible in any language that can express a C struct.
#[repr(C)]
#[derive(Debug, Default, Copy, Clone, PartialEq, Eq, Hash)]
pub struct blink_keyboard_info {
    /// A `blink_model` value.
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
pub struct blink_keyboard {
    _unused: [u8; 0],
}

#[repr(C)]
#[derive(Debug, Copy, Clone)]
pub struct blink_lcd {
    _unused: [u8; 0],
}

/* ----------------------------------------------------------------------- */
/* Injectable USB transport                                                */
/* ----------------------------------------------------------------------- */

/// A caller-supplied USB backend. Every callback receives the `user_data`
/// pointer from the containing struct as its first argument.
///
/// A NULL callback is treated as an unimplemented operation and makes the
/// corresponding library call fail with `BLINK_ERR_UNSUPPORTED`. Callbacks must
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
pub struct blink_usb_io {
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

    pub fn blink_abi_version() -> u32;

    pub fn blink_version_string() -> *const c_char;

    /* ------------------------------------------------------------------- */
    /* Errors                                                              */
    /* ------------------------------------------------------------------- */

    pub fn blink_last_error_message() -> *const c_char;

    pub fn blink_clear_last_error();

    pub fn blink_status_name(status: i32) -> *const c_char;

    /* ------------------------------------------------------------------- */
    /* Device enumeration                                                  */
    /* ------------------------------------------------------------------- */

    pub fn blink_list_known_keyboards(
        out_infos: *mut blink_keyboard_info,
        capacity: usize,
        out_count: *mut usize,
    ) -> blink_status;

    pub fn blink_list_available_keyboards(
        out_infos: *mut blink_keyboard_info,
        capacity: usize,
        out_count: *mut usize,
    ) -> blink_status;

    pub fn blink_model_get_info(
        model: blink_model,
        out_info: *mut blink_keyboard_info,
    ) -> blink_status;

    /* ------------------------------------------------------------------- */
    /* Key name introspection                                              */
    /* ------------------------------------------------------------------- */

    pub fn blink_model_key_count(model: blink_model, out_count: *mut usize) -> blink_status;

    pub fn blink_model_key_at(
        model: blink_model,
        index: usize,
        out_name: *mut *const c_char,
        out_key_id: *mut u8,
    ) -> blink_status;

    pub fn blink_model_key_id_for_name(
        model: blink_model,
        name: *const c_char,
        out_key_id: *mut u8,
    ) -> blink_status;

    /* ------------------------------------------------------------------- */
    /* Keyboard LED control                                                */
    /* ------------------------------------------------------------------- */

    pub fn blink_keyboard_open(
        model: blink_model,
        out_keyboard: *mut *mut blink_keyboard,
    ) -> blink_status;

    pub fn blink_keyboard_close(keyboard: *mut blink_keyboard);

    pub fn blink_keyboard_connect(keyboard: *mut blink_keyboard) -> blink_status;

    pub fn blink_keyboard_is_connected(
        keyboard: *const blink_keyboard,
        out_connected: *mut i32,
    ) -> blink_status;

    pub fn blink_keyboard_get_info(
        keyboard: *const blink_keyboard,
        out_info: *mut blink_keyboard_info,
    ) -> blink_status;

    pub fn blink_keyboard_set_active_key_ids(
        keyboard: *mut blink_keyboard,
        key_ids: *const u8,
        count: usize,
    ) -> blink_status;

    pub fn blink_keyboard_set_active_key_names(
        keyboard: *mut blink_keyboard,
        names: *const *const c_char,
        count: usize,
    ) -> blink_status;

    pub fn blink_keyboard_get_active_key_count(
        keyboard: *const blink_keyboard,
        out_count: *mut usize,
    ) -> blink_status;

    pub fn blink_keyboard_set_keys(
        keyboard: *mut blink_keyboard,
        value: blink_key_value,
    ) -> blink_status;

    pub fn blink_keyboard_set_keys_capture(
        keyboard: *mut blink_keyboard,
        value: blink_key_value,
        out_messages: *mut u8,
        capacity: usize,
        out_written: *mut usize,
    ) -> blink_status;

    pub fn blink_keyboard_set_keys_rgb(
        keyboard: *mut blink_keyboard,
        r: u8,
        g: u8,
        b: u8,
    ) -> blink_status;

    pub fn blink_keyboard_blink(
        keyboard: *mut blink_keyboard,
        count: i32,
        interval_ms: i32,
    ) -> blink_status;

    /* ------------------------------------------------------------------- */
    /* LCD control                                                         */
    /* ------------------------------------------------------------------- */

    pub fn blink_sk80_lcd_defaults(
        out_vid: *mut u16,
        out_pid: *mut u16,
        out_control_interface: *mut *const c_char,
        out_data_interface: *mut *const c_char,
    ) -> blink_status;

    pub fn blink_lcd_open(
        io: *const blink_usb_io,
        vid: u16,
        pid: u16,
        control_interface: *const c_char,
        data_interface: *const c_char,
        out_lcd: *mut *mut blink_lcd,
    ) -> blink_status;

    pub fn blink_lcd_close(lcd: *mut blink_lcd);

    pub fn blink_lcd_connect(lcd: *mut blink_lcd) -> blink_status;

    pub fn blink_lcd_is_connected(lcd: *const blink_lcd, out_connected: *mut i32) -> blink_status;

    pub fn blink_lcd_upload_image(lcd: *mut blink_lcd, image_path: *const c_char) -> blink_status;

    /* ------------------------------------------------------------------- */
    /* Diagnostics                                                         */
    /* ------------------------------------------------------------------- */

    pub fn blink_print_messages(
        buffer: *const u8,
        message_count: usize,
        message_length: usize,
    ) -> blink_status;
}
