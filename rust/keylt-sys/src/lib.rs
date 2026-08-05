//! Raw FFI bindings to the [`keylt`] C ABI for controlling RGB mechanical
//! keyboard LEDs and LCD displays.
//!
//! This crate is a mechanical translation of `keylt.h` and applies no safety
//! rules of its own. Prefer the safe `keylt` crate, which wraps everything here
//! in RAII handles, `Result`s and borrow-checked buffers.
//!
//! # Choosing the native library
//!
//! The build script looks for the native library in this order:
//!
//! 1. `KEYLT_LIB_DIR` and `KEYLT_INCLUDE_DIR`, pointing at an install prefix.
//!    `KEYLT_INCLUDE_DIR` must contain `keylt/keylt.h`.
//! 2. `pkg-config`, using the `keylt.pc` the CMake project installs.
//! 3. The `vendored` feature (on by default), which builds the CMake project at
//!    the repository root and links the resulting static archive.
//!
//! # Features
//!
//! * `vendored` *(default)* -- build the C library from source with CMake.
//!   Implies static linkage.
//! * `static` -- link the static archive when using modes 1 or 2 above. This
//!   also links keylt's own dependencies, because `keylt_static` exposes hidapi
//!   as a public usage requirement.
//! * `bindgen` -- regenerate the declarations from `keylt.h` at build time
//!   instead of using the checked-in ones. Requires libclang.
//!
//! # Safety contract
//!
//! Everything below is inherited from the C header and is *not* enforced here:
//!
//! * A handle must not be used from two threads at once. Distinct handles may
//!   be used from distinct threads.
//! * [`keylt_last_error_message`] returns a thread-local pointer that is only
//!   valid until the next `keylt_*` call on the same thread.
//! * The library never allocates memory for the caller to free. Buffers are
//!   always supplied by the caller with an explicit capacity.
//! * Callbacks installed through [`keylt_usb_io`] must not unwind.
//!
//! [`keylt`]: https://github.com/TheNotary/keyboard_driver_sk80

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![cfg_attr(docsrs, feature(doc_cfg))]

#[cfg(feature = "bindgen")]
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

#[cfg(not(feature = "bindgen"))]
mod bindings;
#[cfg(not(feature = "bindgen"))]
pub use bindings::*;

/// The ABI version this crate was written against, encoded as
/// `(major << 16) | (minor << 8) | patch`.
///
/// The header spells this as a macro that bindgen cannot always fold, so it is
/// recomputed here from the individual components.
pub const KEYLT_ABI_VERSION: u32 =
    (KEYLT_ABI_VERSION_MAJOR << 16) | (KEYLT_ABI_VERSION_MINOR << 8) | KEYLT_ABI_VERSION_PATCH;

impl Default for keylt_usb_io {
    /// An all-NULL vtable. Every operation is reported as unimplemented until
    /// the individual callbacks are filled in.
    fn default() -> Self {
        Self {
            user_data: std::ptr::null_mut(),
            open: None,
            send_feature_report: None,
            get_feature_report: None,
            write_data: None,
            read_data: None,
            close: None,
        }
    }
}

/// Returns `true` when the loaded library's ABI major version matches the one
/// this crate was compiled against.
///
/// The major version is the only component that governs compatibility: the C
/// ABI adds functions in minor releases and never reorders or removes them.
pub fn abi_is_compatible() -> bool {
    // SAFETY: takes no arguments, touches no state and never fails.
    let loaded = unsafe { keylt_abi_version() };
    (loaded >> 16) == (KEYLT_ABI_VERSION >> 16)
}
