//! An owned handle to a keyboard's LEDs.

use std::ffi::CString;
use std::os::raw::c_char;
use std::ptr::NonNull;
use std::time::Duration;

use blink_sys as sys;

use crate::error::{check, Error, Result, Status};
use crate::model::{KeyState, KeyboardInfo, Model, Rgb};

/// A connection to one keyboard's LEDs.
///
/// Opening a handle does not touch hardware; call [`Keyboard::connect`] before
/// any operation that talks to the device. The handle is closed when it is
/// dropped.
///
/// # Threading
///
/// A handle may be moved between threads but must not be used from two at once,
/// which is exactly `Send` without `Sync`. The library takes no locks and
/// creates no threads.
///
/// # Examples
///
/// ```no_run
/// use blink::{Keyboard, KeyState, Model};
///
/// let mut keyboard = Keyboard::open(Model::Sk80)?;
/// keyboard.connect()?;
/// keyboard.set_active_key_names(&["esc", "f1", "f2"])?;
/// keyboard.set_keys(KeyState::On)?;
/// # Ok::<(), blink::Error>(())
/// ```
pub struct Keyboard {
    handle: NonNull<sys::blink_keyboard>,
    model: Model,
}

// SAFETY: the C ABI documents that distinct handles may be used from distinct
// threads, and that the last-error slot is thread-local. `Sync` is deliberately
// not implemented, because a single handle must not be used concurrently.
unsafe impl Send for Keyboard {}

impl Keyboard {
    /// Creates a handle for `model` without touching hardware.
    ///
    /// # Errors
    ///
    /// Returns [`Status::Unsupported`] for a model the library does not
    /// implement, such as [`Model::Mk84`].
    pub fn open(model: Model) -> Result<Self> {
        let mut handle: *mut sys::blink_keyboard = std::ptr::null_mut();
        // SAFETY: `handle` is a valid, writable out-parameter. On success the
        // library hands over ownership of a handle freed in `Drop`.
        check(unsafe { sys::blink_keyboard_open(model.as_raw(), &mut handle) })?;

        let handle = NonNull::new(handle).ok_or_else(|| {
            Error::local(
                Status::Internal,
                "blink_keyboard_open reported success but returned NULL",
            )
        })?;
        Ok(Self { handle, model })
    }

    /// The model this handle drives.
    #[must_use]
    pub fn model(&self) -> Model {
        self.model
    }

    /// Opens the underlying USB device. Safe to call more than once.
    ///
    /// May block briefly while USB devices are enumerated.
    ///
    /// # Errors
    ///
    /// Returns [`Status::DeviceNotFound`] when no matching keyboard is
    /// attached.
    pub fn connect(&mut self) -> Result<()> {
        // SAFETY: `self.handle` is non-null and owned by `self`.
        check(unsafe { sys::blink_keyboard_connect(self.handle.as_ptr()) })
    }

    /// Whether the USB device is currently open.
    ///
    /// This reports the cached connection state and does not re-enumerate.
    ///
    /// # Errors
    ///
    /// Propagates any failure from the library.
    pub fn is_connected(&self) -> Result<bool> {
        let mut connected: i32 = 0;
        // SAFETY: the handle is valid and `connected` is writable.
        check(unsafe { sys::blink_keyboard_is_connected(self.handle.as_ptr(), &mut connected) })?;
        Ok(connected != 0)
    }

    /// Describes the model behind this handle.
    ///
    /// # Errors
    ///
    /// Propagates any failure from the library.
    pub fn info(&self) -> Result<KeyboardInfo> {
        let mut raw = sys::blink_keyboard_info::default();
        // SAFETY: the handle is valid and `raw` is writable.
        check(unsafe { sys::blink_keyboard_get_info(self.handle.as_ptr(), &mut raw) })?;
        KeyboardInfo::from_raw(&raw)
    }

    /// Selects the keys that subsequent `set_*` calls act on, by hardware id.
    ///
    /// # Errors
    ///
    /// Returns [`Status::OutOfRange`] if any id exceeds the model's
    /// `max_key_id`. Validation is all-or-nothing: on failure the previous
    /// selection is left intact.
    pub fn set_active_key_ids(&mut self, key_ids: &[u8]) -> Result<()> {
        // An empty slice still yields a non-null dangling pointer, which the
        // ABI accepts because it only reads `count` elements.
        // SAFETY: `key_ids` is valid for `len` reads for the duration of the
        // call.
        check(unsafe {
            sys::blink_keyboard_set_active_key_ids(
                self.handle.as_ptr(),
                key_ids.as_ptr(),
                key_ids.len(),
            )
        })
    }

    /// Selects the keys that subsequent `set_*` calls act on, by name.
    ///
    /// Names are resolved through the model's key table; see [`Model::keys`]
    /// for the full list. Matching is case-sensitive.
    ///
    /// # Errors
    ///
    /// Returns [`Status::InvalidArgument`] if any name is unknown to the model
    /// or contains an interior NUL byte. On failure the previous selection is
    /// left intact.
    pub fn set_active_key_names<S: AsRef<str>>(&mut self, names: &[S]) -> Result<()> {
        // The CStrings must outlive the call, so they are collected before the
        // pointer array is built.
        let owned: Vec<CString> = names
            .iter()
            .map(|name| CString::new(name.as_ref()).map_err(Error::from))
            .collect::<Result<_>>()?;
        let pointers: Vec<*const c_char> = owned.iter().map(|name| name.as_ptr()).collect();

        // SAFETY: `pointers` holds `len` valid NUL-terminated strings, all kept
        // alive by `owned` until the call returns.
        check(unsafe {
            sys::blink_keyboard_set_active_key_names(
                self.handle.as_ptr(),
                pointers.as_ptr(),
                pointers.len(),
            )
        })
    }

    /// Clears the active key selection.
    ///
    /// # Errors
    ///
    /// Propagates any failure from the library.
    pub fn clear_active_keys(&mut self) -> Result<()> {
        self.set_active_key_ids(&[])
    }

    /// How many keys are currently selected.
    ///
    /// # Errors
    ///
    /// Propagates any failure from the library.
    pub fn active_key_count(&self) -> Result<usize> {
        let mut count = 0usize;
        // SAFETY: the handle is valid and `count` is writable.
        check(unsafe {
            sys::blink_keyboard_get_active_key_count(self.handle.as_ptr(), &mut count)
        })?;
        Ok(count)
    }

    /// Drives the active keys to `state`.
    ///
    /// # Errors
    ///
    /// Returns [`Status::DeviceNotFound`] if the keyboard is not connected.
    pub fn set_keys(&mut self, state: KeyState) -> Result<()> {
        // SAFETY: the handle is valid and `state` maps to a defined enumerator.
        check(unsafe { sys::blink_keyboard_set_keys(self.handle.as_ptr(), state.as_raw()) })
    }

    /// Lights the active keys.
    ///
    /// # Errors
    ///
    /// Returns [`Status::DeviceNotFound`] if the keyboard is not connected.
    pub fn turn_on(&mut self) -> Result<()> {
        self.set_keys(KeyState::On)
    }

    /// Darkens the active keys.
    ///
    /// # Errors
    ///
    /// Returns [`Status::DeviceNotFound`] if the keyboard is not connected.
    pub fn turn_off(&mut self) -> Result<()> {
        self.set_keys(KeyState::Off)
    }

    /// As [`Keyboard::set_keys`], but also copies the packets that were sent
    /// into `out`, returning how many bytes were written.
    ///
    /// The required capacity is [`KeyboardInfo::capture_buffer_len`].
    ///
    /// # Errors
    ///
    /// Returns [`Status::BufferTooSmall`] when `out` is too short, in which case
    /// nothing is sent to the device. Use
    /// [`Keyboard::set_keys_capture_to_vec`] to have the buffer sized for you.
    pub fn set_keys_capture(&mut self, state: KeyState, out: &mut [u8]) -> Result<usize> {
        let mut written = 0usize;
        // SAFETY: `out` is valid for `len` writes and `written` is writable.
        let status = unsafe {
            sys::blink_keyboard_set_keys_capture(
                self.handle.as_ptr(),
                state.as_raw(),
                out.as_mut_ptr(),
                out.len(),
                &mut written,
            )
        };
        check(status)?;
        Ok(written)
    }

    /// As [`Keyboard::set_keys`], returning the packets that were sent.
    ///
    /// The buffer is sized from the model's info, and resized once if the
    /// library reports it needs more. That retry re-sends, so it is only
    /// reachable if the model's advertised packet geometry is wrong.
    ///
    /// # Errors
    ///
    /// Returns [`Status::DeviceNotFound`] if the keyboard is not connected.
    pub fn set_keys_capture_to_vec(&mut self, state: KeyState) -> Result<Vec<u8>> {
        let mut buffer = vec![0u8; self.info()?.capture_buffer_len()];
        match self.set_keys_capture(state, &mut buffer) {
            Ok(written) => {
                buffer.truncate(written);
                Ok(buffer)
            }
            Err(err) if err.status() == Status::BufferTooSmall => {
                // On BUFFER_TOO_SMALL the library writes the required size into
                // `out_written` and sends nothing, so retrying is safe.
                let required = self.required_capture_len(state)?;
                let mut buffer = vec![0u8; required];
                let written = self.set_keys_capture(state, &mut buffer)?;
                buffer.truncate(written);
                Ok(buffer)
            }
            Err(err) => Err(err),
        }
    }

    /// Asks the library how large a capture buffer this model needs, by
    /// deliberately offering none.
    fn required_capture_len(&mut self, state: KeyState) -> Result<usize> {
        let mut written = 0usize;
        let mut empty = [0u8; 0];
        // SAFETY: a zero capacity is honest about the empty buffer, so the
        // library reports the requirement without writing anything.
        let status = unsafe {
            sys::blink_keyboard_set_keys_capture(
                self.handle.as_ptr(),
                state.as_raw(),
                empty.as_mut_ptr(),
                0,
                &mut written,
            )
        };
        if status == sys::BLINK_ERR_BUFFER_TOO_SMALL || status == sys::BLINK_OK {
            Ok(written)
        } else {
            Err(Error::from_raw(status))
        }
    }

    /// Sets the active keys to an RGB colour.
    ///
    /// # Errors
    ///
    /// Returns [`Status::Unsupported`] for a model that does not implement RGB,
    /// and [`Status::DeviceNotFound`] if the keyboard is not connected.
    pub fn set_keys_rgb(&mut self, colour: impl Into<Rgb>) -> Result<()> {
        let Rgb { r, g, b } = colour.into();
        // SAFETY: the handle is valid; all byte values are in range.
        check(unsafe { sys::blink_keyboard_set_keys_rgb(self.handle.as_ptr(), r, g, b) })
    }

    /// Cycles the active keys through a fixed colour sequence `count` times.
    ///
    /// # Blocking
    ///
    /// This stalls the calling thread for roughly `count * interval * 7` and
    /// cannot be cancelled. Call it from a thread that can afford to wait.
    ///
    /// # Errors
    ///
    /// Returns [`Status::InvalidArgument`] if `count` or `interval` exceeds
    /// what the C ABI can represent, and [`Status::DeviceNotFound`] if the
    /// keyboard is not connected.
    pub fn blink(&mut self, count: u32, interval: Duration) -> Result<()> {
        let count = i32::try_from(count).map_err(|_| {
            Error::local(
                Status::InvalidArgument,
                format!("blink count {count} does not fit in an i32"),
            )
        })?;
        let interval_ms = i32::try_from(interval.as_millis()).map_err(|_| {
            Error::local(
                Status::InvalidArgument,
                format!(
                    "blink interval of {} ms does not fit in an i32",
                    interval.as_millis()
                ),
            )
        })?;
        // SAFETY: the handle is valid and both counts are non-negative.
        check(unsafe { sys::blink_keyboard_blink(self.handle.as_ptr(), count, interval_ms) })
    }
}

impl Drop for Keyboard {
    fn drop(&mut self) {
        // SAFETY: the sole destroy path for a handle this type owns. It is
        // never called twice, because `Keyboard` is not `Copy` or `Clone`.
        unsafe { sys::blink_keyboard_close(self.handle.as_ptr()) }
    }
}

impl std::fmt::Debug for Keyboard {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Keyboard")
            .field("model", &self.model)
            .field("connected", &self.is_connected().unwrap_or(false))
            .finish_non_exhaustive()
    }
}
