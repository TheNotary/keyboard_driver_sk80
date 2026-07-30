//! Safe Rust bindings for controlling the LEDs and LCD of RGB mechanical
//! keyboards.
//!
//! This wraps the `blink` C ABI: opaque handles become RAII types, status codes
//! become [`Result`]s, and the caller-allocated buffer protocols are hidden
//! behind ordinary slices and `Vec`s. Nothing here allocates on the library's
//! behalf, because the library never asks it to.
//!
//! # Quick start
//!
//! ```no_run
//! use blink::{KeyState, Keyboard, Model};
//!
//! # fn main() -> Result<(), blink::Error> {
//! for info in blink::list_available_keyboards()? {
//!     println!("{} at {:04x}:{:04x}", info.display_name, info.vid, info.pid);
//! }
//!
//! let mut keyboard = Keyboard::open(Model::Sk80)?;
//! keyboard.connect()?;
//! keyboard.set_active_key_names(&["esc", "f1", "f2"])?;
//! keyboard.set_keys(KeyState::On)?;
//! # Ok(())
//! # }
//! ```
//!
//! # Linking
//!
//! The native library is located by [`blink-sys`](blink_sys), which by default
//! builds it from source with `CMake`. Set `BLINK_LIB_DIR` and
//! `BLINK_INCLUDE_DIR`, or install `blink.pc` where pkg-config can see it, to
//! link an existing build instead.
//!
//! # Threading
//!
//! [`Keyboard`] and [`Lcd`] are `Send` but not `Sync`: a handle may be moved
//! between threads, but the library takes no locks, so a single handle must not
//! be used from two threads at once. Error messages are recorded per-thread and
//! captured into [`Error`] before this crate returns.
//!
//! # Blocking
//!
//! [`Keyboard::blink`] and [`Lcd::upload_image`] stall the calling thread for a
//! long time and cannot be cancelled.

#![deny(missing_docs)]
#![warn(clippy::pedantic)]
#![allow(clippy::missing_errors_doc)]
#![cfg_attr(docsrs, feature(doc_cfg))]

mod error;
mod keyboard;
mod lcd;
mod model;
mod usb_io;

pub use error::{clear_last_error, Error, Result, Status};
pub use keyboard::Keyboard;
pub use lcd::{sk80_lcd_defaults, Lcd, LcdDefaults};
pub use model::{
    abi_is_compatible, abi_version, list_available_keyboards, list_known_keyboards, version, Key,
    KeyState, KeyboardInfo, Model, Rgb,
};
pub use usb_io::UsbIo;

/// Re-export of the raw bindings, for the rare call this crate does not cover.
pub use blink_sys as sys;

/// Prints raw protocol packets to stdout as hexadecimal.
///
/// `buffer` is read as `message_count` packets of `message_length` bytes. This
/// is a debugging aid for captures taken with
/// [`Keyboard::set_keys_capture`]; it writes to the library's stdout, not
/// Rust's.
///
/// # Errors
///
/// Returns [`Status::InvalidArgument`] if `buffer` is shorter than
/// `message_count * message_length`.
pub fn print_messages(buffer: &[u8], message_count: usize, message_length: usize) -> Result<()> {
    // The ABI trusts the caller's arithmetic, so check it here rather than
    // letting the library read past the end of the slice.
    let required = message_count
        .checked_mul(message_length)
        .ok_or_else(|| Error::local(Status::InvalidArgument, "message geometry overflows"))?;
    if buffer.len() < required {
        return Err(Error::local(
            Status::InvalidArgument,
            format!(
                "buffer holds {} bytes but {message_count} messages of {message_length} bytes need {required}",
                buffer.len()
            ),
        ));
    }

    // SAFETY: `buffer` is valid for `required` reads, which is all the library
    // will touch.
    error::check(unsafe {
        sys::blink_print_messages(buffer.as_ptr(), message_count, message_length)
    })
}
