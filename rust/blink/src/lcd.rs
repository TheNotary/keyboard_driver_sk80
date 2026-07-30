//! An owned handle to the SK80's LCD panel.

use std::ffi::CString;
use std::path::Path;
use std::ptr::NonNull;

use blink_sys as sys;

use crate::error::{check, Error, Result, Status};
use crate::model::static_str;
use crate::usb_io::{BackendState, UsbIo};

/// The USB identifiers and HID interface selectors for the SK80's LCD.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct LcdDefaults {
    /// USB vendor id.
    pub vid: u16,
    /// USB product id.
    pub pid: u16,
    /// HID interface selector used for control traffic.
    pub control_interface: &'static str,
    /// HID interface selector used for bulk image data.
    pub data_interface: &'static str,
}

/// Reports the identifiers for the SK80's LCD, so they need not be hard-coded.
///
/// # Errors
///
/// Propagates any failure from the library.
pub fn sk80_lcd_defaults() -> Result<LcdDefaults> {
    let mut vid: u16 = 0;
    let mut pid: u16 = 0;
    let mut control = std::ptr::null();
    let mut data = std::ptr::null();
    // SAFETY: all four out-parameters are valid and writable. The two string
    // pointers receive static storage owned by the library.
    check(unsafe { sys::blink_sk80_lcd_defaults(&mut vid, &mut pid, &mut control, &mut data) })?;
    Ok(LcdDefaults {
        vid,
        pid,
        control_interface: static_str(control)?,
        data_interface: static_str(data)?,
    })
}

/// A connection to a keyboard's LCD panel.
///
/// Opening a handle does not touch hardware; call [`Lcd::connect`] first. The
/// handle is closed when it is dropped.
///
/// # Threading
///
/// As with [`Keyboard`](crate::Keyboard), a handle may move between threads but
/// must not be used from two at once: `Send` without `Sync`.
///
/// # Examples
///
/// ```no_run
/// let mut lcd = blink::Lcd::open_sk80()?;
/// lcd.connect()?;
/// lcd.upload_image("splash.gif")?;
/// # Ok::<(), blink::Error>(())
/// ```
pub struct Lcd {
    handle: NonNull<sys::blink_lcd>,
    /// Kept alive for as long as the handle: the library copies the vtable but
    /// keeps calling back through this pointer. `Drop` closes the handle before
    /// this field is dropped.
    backend: Option<Box<BackendState>>,
}

// SAFETY: as for `Keyboard`, distinct handles may be used from distinct threads.
// `UsbIo` is itself `Send`, so a handle carrying a backend is no different.
unsafe impl Send for Lcd {}

impl Lcd {
    /// Opens the SK80's LCD using the library's built-in USB backend.
    ///
    /// # Errors
    ///
    /// Propagates any failure from the library.
    pub fn open_sk80() -> Result<Self> {
        let defaults = sk80_lcd_defaults()?;
        Self::open(
            defaults.vid,
            defaults.pid,
            defaults.control_interface,
            defaults.data_interface,
        )
    }

    /// Opens the SK80's LCD using a caller-supplied transport.
    ///
    /// # Errors
    ///
    /// Propagates any failure from the library.
    pub fn open_sk80_with<T: UsbIo + 'static>(io: T) -> Result<Self> {
        let defaults = sk80_lcd_defaults()?;
        Self::open_with(
            io,
            defaults.vid,
            defaults.pid,
            defaults.control_interface,
            defaults.data_interface,
        )
    }

    /// Opens an LCD using the library's built-in USB backend.
    ///
    /// # Errors
    ///
    /// Returns [`Status::InvalidArgument`] if either interface selector
    /// contains an interior NUL byte.
    pub fn open(vid: u16, pid: u16, control_interface: &str, data_interface: &str) -> Result<Self> {
        Self::open_inner(None, vid, pid, control_interface, data_interface)
    }

    /// Opens an LCD driven by a caller-supplied transport.
    ///
    /// This is the seam for a mock, a recorder, or an alternative HID stack.
    ///
    /// # Errors
    ///
    /// Returns [`Status::InvalidArgument`] if either interface selector
    /// contains an interior NUL byte.
    pub fn open_with<T: UsbIo + 'static>(
        io: T,
        vid: u16,
        pid: u16,
        control_interface: &str,
        data_interface: &str,
    ) -> Result<Self> {
        Self::open_inner(
            Some(Box::new(BackendState::new(Box::new(io)))),
            vid,
            pid,
            control_interface,
            data_interface,
        )
    }

    fn open_inner(
        backend: Option<Box<BackendState>>,
        vid: u16,
        pid: u16,
        control_interface: &str,
        data_interface: &str,
    ) -> Result<Self> {
        let control = CString::new(control_interface)?;
        let data = CString::new(data_interface)?;

        // The vtable borrows the state, which `Self` then owns; the library
        // copies the struct, so it only has to survive this call.
        let mut backend = backend;
        let vtable = backend.as_mut().map(|state| {
            // SAFETY: `state` stays alive inside `Self` for as long as the
            // handle does, and is only dropped after `blink_lcd_close`.
            unsafe { BackendState::vtable(std::ptr::addr_of_mut!(**state)) }
        });

        let mut handle: *mut sys::blink_lcd = std::ptr::null_mut();
        // SAFETY: `vtable`, `control` and `data` all outlive the call, and
        // `handle` is a valid out-parameter. A NULL `io` selects the library's
        // built-in backend.
        let status = unsafe {
            sys::blink_lcd_open(
                vtable.as_ref().map_or(std::ptr::null(), std::ptr::from_ref),
                vid,
                pid,
                control.as_ptr(),
                data.as_ptr(),
                &mut handle,
            )
        };
        check(status)?;

        let handle = NonNull::new(handle).ok_or_else(|| {
            Error::local(
                Status::Internal,
                "blink_lcd_open reported success but returned NULL",
            )
        })?;
        Ok(Self { handle, backend })
    }

    /// Opens the underlying USB device. Safe to call more than once.
    ///
    /// May block briefly while USB devices are enumerated.
    ///
    /// # Errors
    ///
    /// Returns [`Status::DeviceNotFound`] when no matching device is attached.
    pub fn connect(&mut self) -> Result<()> {
        // SAFETY: the handle is non-null and owned by `self`.
        check(unsafe { sys::blink_lcd_connect(self.handle.as_ptr()) })
    }

    /// Whether the USB device is currently open.
    ///
    /// # Errors
    ///
    /// Propagates any failure from the library.
    pub fn is_connected(&self) -> Result<bool> {
        let mut connected: i32 = 0;
        // SAFETY: the handle is valid and `connected` is writable.
        check(unsafe { sys::blink_lcd_is_connected(self.handle.as_ptr(), &mut connected) })?;
        Ok(connected != 0)
    }

    /// Uploads a still image or animated GIF to the display.
    ///
    /// # Blocking
    ///
    /// This takes a long time -- the protocol requires a pause between every
    /// 4 KiB page -- reports no progress, and cannot be cancelled.
    ///
    /// # Errors
    ///
    /// Returns [`Status::InvalidArgument`] if the path is not valid UTF-8 or
    /// contains an interior NUL byte, [`Status::DeviceNotFound`] if the display
    /// is not connected, and [`Status::Io`] if the transfer does not complete.
    pub fn upload_image(&mut self, image_path: impl AsRef<Path>) -> Result<()> {
        let path = image_path.as_ref();
        // The ABI takes a `const char*`, so there is no way to express a path
        // that is not valid UTF-8.
        let path = path.to_str().ok_or_else(|| {
            Error::local(
                Status::InvalidArgument,
                format!("image path is not valid UTF-8: {}", path.display()),
            )
        })?;
        let path = CString::new(path)?;
        // SAFETY: the handle is valid and `path` outlives the call.
        check(unsafe { sys::blink_lcd_upload_image(self.handle.as_ptr(), path.as_ptr()) })
    }
}

impl Drop for Lcd {
    fn drop(&mut self) {
        // SAFETY: the sole destroy path for a handle this type owns. Fields are
        // dropped after this body runs, so the backend outlives the library's
        // last callback.
        unsafe { sys::blink_lcd_close(self.handle.as_ptr()) }
    }
}

impl std::fmt::Debug for Lcd {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Lcd")
            .field("connected", &self.is_connected().unwrap_or(false))
            .field("custom_backend", &self.backend.is_some())
            .finish_non_exhaustive()
    }
}
