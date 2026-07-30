//! Error reporting.
//!
//! Every fallible call in the C ABI returns a negative status code and records a
//! human-readable description in a thread-local slot. [`Error`] pairs the two,
//! copying the message eagerly because the pointer the library hands back is
//! only valid until the next call on that thread.

use std::ffi::{CStr, NulError};
use std::fmt;

use keylt_sys as sys;

/// A shorthand for results produced by this crate.
pub type Result<T> = std::result::Result<T, Error>;

/// A status code returned by the C ABI.
///
/// This is deliberately not a `#[repr(C)]` enum: the library may add codes in a
/// future release, and unlisted discriminants are captured by [`Status::Unknown`]
/// rather than being undefined behaviour.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[non_exhaustive]
pub enum Status {
    /// An argument was NULL, malformed, or named something the model does not
    /// define.
    InvalidArgument,
    /// No matching device is attached, or the handle is not connected.
    DeviceNotFound,
    /// An index or key id was outside the range the model accepts.
    OutOfRange,
    /// The model or operation is recognised but not implemented.
    Unsupported,
    /// The transfer to or from the device failed.
    Io,
    /// The library could not allocate a handle.
    OutOfMemory,
    /// The supplied buffer was smaller than the operation required. Nothing was
    /// sent to the device.
    BufferTooSmall,
    /// An unexpected failure inside the library.
    Internal,
    /// A status code this crate does not know about.
    Unknown(i32),
}

impl Status {
    /// Translates a raw `keylt_status` value.
    #[must_use]
    pub fn from_raw(status: sys::keylt_status) -> Self {
        match status {
            sys::KEYLT_ERR_INVALID_ARGUMENT => Self::InvalidArgument,
            sys::KEYLT_ERR_DEVICE_NOT_FOUND => Self::DeviceNotFound,
            sys::KEYLT_ERR_OUT_OF_RANGE => Self::OutOfRange,
            sys::KEYLT_ERR_UNSUPPORTED => Self::Unsupported,
            sys::KEYLT_ERR_IO => Self::Io,
            sys::KEYLT_ERR_OUT_OF_MEMORY => Self::OutOfMemory,
            sys::KEYLT_ERR_BUFFER_TOO_SMALL => Self::BufferTooSmall,
            sys::KEYLT_ERR_INTERNAL => Self::Internal,
            other => Self::Unknown(other),
        }
    }

    /// The raw `keylt_status` value this status corresponds to.
    #[must_use]
    pub fn as_raw(self) -> sys::keylt_status {
        match self {
            Self::InvalidArgument => sys::KEYLT_ERR_INVALID_ARGUMENT,
            Self::DeviceNotFound => sys::KEYLT_ERR_DEVICE_NOT_FOUND,
            Self::OutOfRange => sys::KEYLT_ERR_OUT_OF_RANGE,
            Self::Unsupported => sys::KEYLT_ERR_UNSUPPORTED,
            Self::Io => sys::KEYLT_ERR_IO,
            Self::OutOfMemory => sys::KEYLT_ERR_OUT_OF_MEMORY,
            Self::BufferTooSmall => sys::KEYLT_ERR_BUFFER_TOO_SMALL,
            Self::Internal => sys::KEYLT_ERR_INTERNAL,
            Self::Unknown(other) => other,
        }
    }

    /// The library's own name for this code, such as `"KEYLT_ERR_IO"`.
    #[must_use]
    pub fn name(self) -> &'static str {
        // SAFETY: `keylt_status_name` accepts any `int32_t` and returns a
        // non-NULL pointer to a string with static storage duration.
        let ptr = unsafe { sys::keylt_status_name(self.as_raw()) };
        if ptr.is_null() {
            return "KEYLT_ERR_UNKNOWN";
        }
        // SAFETY: the returned string is NUL-terminated, static, and ASCII.
        unsafe { CStr::from_ptr(ptr) }
            .to_str()
            .unwrap_or("KEYLT_ERR_UNKNOWN")
    }
}

impl fmt::Display for Status {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(self.name())
    }
}

/// A failed call into the library.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Error {
    status: Status,
    message: String,
}

impl Error {
    /// Builds an error from a raw status, capturing the library's thread-local
    /// message for it.
    ///
    /// This must be called on the same thread as, and immediately after, the
    /// failing call: the message slot is per-thread and is overwritten by the
    /// next `keylt_*` call.
    pub(crate) fn from_raw(status: sys::keylt_status) -> Self {
        Self {
            status: Status::from_raw(status),
            message: last_error_message(),
        }
    }

    /// Builds an error that originated in this crate rather than in the library,
    /// such as a Rust string that cannot be passed to C.
    pub(crate) fn local(status: Status, message: impl Into<String>) -> Self {
        Self {
            status,
            message: message.into(),
        }
    }

    /// The status code the library returned.
    #[must_use]
    pub fn status(&self) -> Status {
        self.status
    }

    /// The library's description of the failure. May be empty.
    #[must_use]
    pub fn message(&self) -> &str {
        &self.message
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.message.is_empty() {
            write!(f, "{}", self.status)
        } else {
            write!(f, "{}: {}", self.status, self.message)
        }
    }
}

impl std::error::Error for Error {}

impl From<NulError> for Error {
    fn from(err: NulError) -> Self {
        Self::local(
            Status::InvalidArgument,
            format!(
                "string contains an interior NUL byte at index {}",
                err.nul_position()
            ),
        )
    }
}

/// Turns a raw status into a `Result`, capturing the error message on failure.
pub(crate) fn check(status: sys::keylt_status) -> Result<()> {
    if status == sys::KEYLT_OK {
        Ok(())
    } else {
        Err(Error::from_raw(status))
    }
}

/// Copies the calling thread's most recent error message.
fn last_error_message() -> String {
    // SAFETY: never returns NULL, and the pointer stays valid until the next
    // `keylt_*` call on this thread. It is copied before returning.
    let ptr = unsafe { sys::keylt_last_error_message() };
    if ptr.is_null() {
        return String::new();
    }
    // SAFETY: the library guarantees a NUL-terminated string.
    unsafe { CStr::from_ptr(ptr) }
        .to_string_lossy()
        .into_owned()
}

/// Discards the calling thread's recorded error message.
///
/// Only useful when inspecting [`Error::message`] across an API boundary that
/// does not go through this crate; every call here captures the message itself.
pub fn clear_last_error() {
    // SAFETY: takes no arguments and only touches thread-local state.
    unsafe { sys::keylt_clear_last_error() }
}
