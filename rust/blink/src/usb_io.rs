//! Supplying your own USB transport.
//!
//! The C ABI accepts a vtable of function pointers in place of its built-in HID
//! backend. [`UsbIo`] is the safe form of that seam: implement it to drive an
//! LCD over a different HID stack, or to record traffic in a test.

use std::ffi::CStr;
use std::io;
use std::os::raw::{c_char, c_void};
use std::panic::{catch_unwind, AssertUnwindSafe};
use std::time::Duration;

use blink_sys as sys;

/// A USB transport the library will use instead of its own.
///
/// Every method mirrors one entry of the C vtable. Returning an error maps to
/// the negative return value the ABI expects; the library then surfaces it as
/// [`Status::Io`](crate::Status::Io) or
/// [`Status::Unsupported`](crate::Status::Unsupported) depending on the call.
///
/// The default method bodies report "unimplemented", matching the ABI's
/// treatment of a NULL callback, so an implementation only has to provide the
/// operations it actually supports.
pub trait UsbIo: Send {
    /// Opens the device. Returns `false` if it could not be opened.
    fn open(&mut self, vid: u16, pid: u16, control_interface: &str, data_interface: &str) -> bool {
        let _ = (vid, pid, control_interface, data_interface);
        false
    }

    /// Sends a HID feature report, returning the number of bytes transferred.
    fn send_feature_report(&mut self, data: &[u8]) -> io::Result<usize> {
        let _ = data;
        Err(unsupported("send_feature_report"))
    }

    /// Reads a HID feature report into `buffer`, returning the number of bytes
    /// transferred.
    fn get_feature_report(&mut self, buffer: &mut [u8]) -> io::Result<usize> {
        let _ = buffer;
        Err(unsupported("get_feature_report"))
    }

    /// Writes to the data interface, returning the number of bytes transferred.
    fn write_data(&mut self, data: &[u8]) -> io::Result<usize> {
        let _ = data;
        Err(unsupported("write_data"))
    }

    /// Reads from the data interface, returning the number of bytes
    /// transferred. A zero-length result means the read timed out.
    fn read_data(&mut self, buffer: &mut [u8], timeout: Duration) -> io::Result<usize> {
        let _ = (buffer, timeout);
        Err(unsupported("read_data"))
    }

    /// Closes the device. Called at most once, before the handle is destroyed.
    fn close(&mut self) {}
}

fn unsupported(what: &str) -> io::Error {
    io::Error::new(
        io::ErrorKind::Unsupported,
        format!("UsbIo::{what} is not implemented"),
    )
}

/// Owns a boxed backend behind a thin pointer, so it can travel through the
/// ABI's `void* user_data`.
pub(crate) struct BackendState {
    backend: Box<dyn UsbIo>,
}

impl BackendState {
    pub(crate) fn new(backend: Box<dyn UsbIo>) -> Self {
        Self { backend }
    }

    /// Builds the vtable the library will copy into its handle.
    ///
    /// # Safety
    ///
    /// `state` must point at a live [`BackendState`] that outlives the LCD
    /// handle the vtable is installed on.
    pub(crate) unsafe fn vtable(state: *mut Self) -> sys::blink_usb_io {
        sys::blink_usb_io {
            user_data: state.cast::<c_void>(),
            open: Some(trampoline_open),
            send_feature_report: Some(trampoline_send_feature_report),
            get_feature_report: Some(trampoline_get_feature_report),
            write_data: Some(trampoline_write_data),
            read_data: Some(trampoline_read_data),
            close: Some(trampoline_close),
        }
    }
}

/// The negative value handed back to C when a callback fails or panics.
const CALLBACK_FAILED: i32 = -1;

/// Runs a callback body, converting a panic into a plain error return.
///
/// Unwinding across the FFI boundary is undefined behaviour, and the header
/// states outright that callbacks must not unwind, so every trampoline goes
/// through here.
fn guard<T>(fallback: T, body: impl FnOnce() -> T) -> T {
    catch_unwind(AssertUnwindSafe(body)).unwrap_or(fallback)
}

/// Recovers the backend from the `user_data` pointer.
///
/// # Safety
///
/// `user_data` must be the pointer installed by [`BackendState::vtable`], and
/// no other reference to the state may be alive. The ABI calls back
/// synchronously from the thread that owns the handle, and `Lcd` is not `Sync`,
/// so that is guaranteed.
unsafe fn backend<'a>(user_data: *mut c_void) -> Option<&'a mut dyn UsbIo> {
    let state = user_data.cast::<BackendState>().as_mut()?;
    Some(state.backend.as_mut())
}

/// Clamps a Rust byte count to what the ABI's `int32_t` can carry.
fn transferred(result: &io::Result<usize>) -> i32 {
    match result {
        Ok(count) => i32::try_from(*count).unwrap_or(i32::MAX),
        Err(_) => CALLBACK_FAILED,
    }
}

/// Borrows a C string as UTF-8, substituting replacement characters rather than
/// failing: interface selectors are ASCII in practice.
///
/// # Safety
///
/// `ptr` must be NULL or a valid NUL-terminated string.
unsafe fn borrow_str(ptr: *const c_char) -> String {
    if ptr.is_null() {
        return String::new();
    }
    CStr::from_ptr(ptr).to_string_lossy().into_owned()
}

unsafe extern "C" fn trampoline_open(
    user_data: *mut c_void,
    vid: u16,
    pid: u16,
    control_interface: *const c_char,
    data_interface: *const c_char,
) -> i32 {
    guard(0, || {
        let Some(backend) = backend(user_data) else {
            return 0;
        };
        let control = borrow_str(control_interface);
        let data = borrow_str(data_interface);
        i32::from(backend.open(vid, pid, &control, &data))
    })
}

unsafe extern "C" fn trampoline_send_feature_report(
    user_data: *mut c_void,
    data: *const u8,
    length: usize,
) -> i32 {
    guard(CALLBACK_FAILED, || {
        let Some(backend) = backend(user_data) else {
            return CALLBACK_FAILED;
        };
        // `from_raw_parts` requires a non-null, aligned pointer even for an
        // empty slice, which C does not promise.
        let slice: &[u8] = if length == 0 || data.is_null() {
            &[]
        } else {
            std::slice::from_raw_parts(data, length)
        };
        transferred(&backend.send_feature_report(slice))
    })
}

unsafe extern "C" fn trampoline_get_feature_report(
    user_data: *mut c_void,
    buffer: *mut u8,
    length: usize,
) -> i32 {
    guard(CALLBACK_FAILED, || {
        let Some(backend) = backend(user_data) else {
            return CALLBACK_FAILED;
        };
        let slice: &mut [u8] = if length == 0 || buffer.is_null() {
            &mut []
        } else {
            std::slice::from_raw_parts_mut(buffer, length)
        };
        transferred(&backend.get_feature_report(slice))
    })
}

unsafe extern "C" fn trampoline_write_data(
    user_data: *mut c_void,
    data: *const u8,
    length: usize,
) -> i32 {
    guard(CALLBACK_FAILED, || {
        let Some(backend) = backend(user_data) else {
            return CALLBACK_FAILED;
        };
        let slice: &[u8] = if length == 0 || data.is_null() {
            &[]
        } else {
            std::slice::from_raw_parts(data, length)
        };
        transferred(&backend.write_data(slice))
    })
}

unsafe extern "C" fn trampoline_read_data(
    user_data: *mut c_void,
    buffer: *mut u8,
    length: usize,
    timeout_ms: i32,
) -> i32 {
    guard(CALLBACK_FAILED, || {
        let Some(backend) = backend(user_data) else {
            return CALLBACK_FAILED;
        };
        let slice: &mut [u8] = if length == 0 || buffer.is_null() {
            &mut []
        } else {
            std::slice::from_raw_parts_mut(buffer, length)
        };
        let timeout = Duration::from_millis(u64::try_from(timeout_ms).unwrap_or(0));
        transferred(&backend.read_data(slice, timeout))
    })
}

unsafe extern "C" fn trampoline_close(user_data: *mut c_void) {
    guard((), || {
        if let Some(backend) = backend(user_data) {
            backend.close();
        }
    });
}
