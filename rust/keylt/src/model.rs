//! Keyboard models, their key tables, and enumeration of attached hardware.

use std::ffi::{CStr, CString};
use std::os::raw::c_char;

use keylt_sys as sys;

use crate::error::{check, Error, Result, Status};

/// A keyboard model the library knows how to drive.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, PartialOrd, Ord)]
#[non_exhaustive]
pub enum Model {
    /// Womier SK80, including its LCD.
    Sk80,
    /// Recognised by the library but not implemented; every call involving it
    /// fails with [`Status::Unsupported`].
    Mk84,
    /// Royal Kludge RK84.
    Rk84,
}

impl Model {
    /// Every model this crate can name.
    pub const ALL: [Model; 3] = [Model::Sk80, Model::Mk84, Model::Rk84];

    /// The raw `keylt_model` value.
    #[must_use]
    pub fn as_raw(self) -> sys::keylt_model {
        match self {
            Self::Sk80 => sys::KEYLT_MODEL_SK80,
            Self::Mk84 => sys::KEYLT_MODEL_MK84,
            Self::Rk84 => sys::KEYLT_MODEL_RK84,
        }
    }

    /// Translates a raw model value, returning `None` for one this crate does
    /// not know.
    #[must_use]
    pub fn from_raw(raw: i32) -> Option<Self> {
        match u32::try_from(raw).ok()? {
            sys::KEYLT_MODEL_SK80 => Some(Self::Sk80),
            sys::KEYLT_MODEL_MK84 => Some(Self::Mk84),
            sys::KEYLT_MODEL_RK84 => Some(Self::Rk84),
            _ => None,
        }
    }

    /// Describes the model without touching hardware.
    ///
    /// # Errors
    ///
    /// Returns [`Status::Unsupported`] for a model the library does not
    /// implement, such as [`Model::Mk84`].
    pub fn info(self) -> Result<KeyboardInfo> {
        let mut raw = sys::keylt_keyboard_info::default();
        // SAFETY: `raw` is a valid, writable `keylt_keyboard_info`.
        check(unsafe { sys::keylt_model_get_info(self.as_raw(), &mut raw) })?;
        KeyboardInfo::from_raw(&raw)
    }

    /// The number of distinct key names the model understands.
    ///
    /// # Errors
    ///
    /// Returns [`Status::Unsupported`] for an unimplemented model.
    pub fn key_count(self) -> Result<usize> {
        let mut count = 0usize;
        // SAFETY: `count` is a valid, writable `size_t`.
        check(unsafe { sys::keylt_model_key_count(self.as_raw(), &mut count) })?;
        Ok(count)
    }

    /// The key at a stable, zero-based index into the model's sorted key table.
    ///
    /// Indices are stable for the lifetime of the process.
    ///
    /// # Errors
    ///
    /// Returns [`Status::OutOfRange`] when `index` is at or past
    /// [`Model::key_count`].
    pub fn key_at(self, index: usize) -> Result<Key> {
        let mut name: *const c_char = std::ptr::null();
        let mut id: u8 = 0;
        // SAFETY: both out-parameters are valid and writable. `out_name`
        // receives a pointer into the library's static key tables.
        check(unsafe { sys::keylt_model_key_at(self.as_raw(), index, &mut name, &mut id) })?;
        Ok(Key {
            name: static_str(name)?,
            id,
        })
    }

    /// The model's entire key table, in index order.
    ///
    /// # Errors
    ///
    /// Returns [`Status::Unsupported`] for an unimplemented model.
    pub fn keys(self) -> Result<Vec<Key>> {
        let count = self.key_count()?;
        (0..count).map(|index| self.key_at(index)).collect()
    }

    /// Resolves a key name to its hardware key id.
    ///
    /// Matching is case-sensitive.
    ///
    /// # Errors
    ///
    /// Returns [`Status::InvalidArgument`] when the model does not define
    /// `name`, or when `name` contains an interior NUL byte.
    pub fn key_id_for_name(self, name: &str) -> Result<u8> {
        let name = CString::new(name)?;
        let mut id: u8 = 0;
        // SAFETY: `name` is a valid NUL-terminated string that outlives the
        // call, and `id` is writable.
        check(unsafe { sys::keylt_model_key_id_for_name(self.as_raw(), name.as_ptr(), &mut id) })?;
        Ok(id)
    }
}

/// One entry of a model's key table.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Key {
    /// The name the model uses for this key, such as `"esc"` or `"f12"`.
    ///
    /// Borrowed from the library's static key tables, so it lives as long as
    /// the process.
    pub name: &'static str,
    /// The hardware key id.
    pub id: u8,
}

/// Whether a key's LED should be lit.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum KeyState {
    /// The LED is dark.
    Off,
    /// The LED is lit.
    On,
}

impl KeyState {
    /// The raw `keylt_key_value`.
    ///
    /// Note that the C ABI uses `OFF = 0, ON = 1`, which is the opposite of the
    /// library's internal enum. The mapping is written out rather than cast.
    #[must_use]
    pub fn as_raw(self) -> sys::keylt_key_value {
        match self {
            Self::Off => sys::KEYLT_KEY_OFF,
            Self::On => sys::KEYLT_KEY_ON,
        }
    }
}

impl From<bool> for KeyState {
    fn from(on: bool) -> Self {
        if on {
            Self::On
        } else {
            Self::Off
        }
    }
}

/// A 24-bit colour.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Default)]
pub struct Rgb {
    /// Red component.
    pub r: u8,
    /// Green component.
    pub g: u8,
    /// Blue component.
    pub b: u8,
}

impl Rgb {
    /// Builds a colour from its components.
    #[must_use]
    pub const fn new(r: u8, g: u8, b: u8) -> Self {
        Self { r, g, b }
    }
}

impl From<(u8, u8, u8)> for Rgb {
    fn from((r, g, b): (u8, u8, u8)) -> Self {
        Self::new(r, g, b)
    }
}

impl From<[u8; 3]> for Rgb {
    fn from([r, g, b]: [u8; 3]) -> Self {
        Self::new(r, g, b)
    }
}

/// Everything the library knows about one keyboard model.
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct KeyboardInfo {
    /// The model this describes.
    pub model: Model,
    /// USB vendor id.
    pub vid: u16,
    /// USB product id.
    pub pid: u16,
    /// The largest key id the model accepts.
    pub max_key_id: u8,
    /// Bytes per USB packet.
    pub message_length: u8,
    /// Packets in one bulk LED update.
    pub bulk_led_value_messages_count: u8,
    /// Human-readable name, such as `"SK80"`.
    pub display_name: String,
}

impl KeyboardInfo {
    /// The buffer size
    /// [`Keyboard::set_keys_capture`](crate::Keyboard::set_keys_capture)
    /// requires for this model.
    #[must_use]
    pub fn capture_buffer_len(&self) -> usize {
        usize::from(self.message_length) * usize::from(self.bulk_led_value_messages_count)
    }

    pub(crate) fn from_raw(raw: &sys::keylt_keyboard_info) -> Result<Self> {
        let model = Model::from_raw(raw.model).ok_or_else(|| {
            Error::local(
                Status::Unsupported,
                format!("library reported unknown keyboard model {}", raw.model),
            )
        })?;
        Ok(Self {
            model,
            vid: raw.vid,
            pid: raw.pid,
            max_key_id: raw.max_key_id,
            message_length: raw.message_length,
            bulk_led_value_messages_count: raw.bulk_led_value_messages_count,
            display_name: decode_fixed_string(&raw.display_name),
        })
    }
}

/// Every model the library can drive, attached or not.
///
/// # Errors
///
/// Propagates any failure from the underlying enumeration.
pub fn list_known_keyboards() -> Result<Vec<KeyboardInfo>> {
    // SAFETY: forwards a caller-owned buffer with an honest capacity.
    list_with(|infos, capacity, count| unsafe {
        sys::keylt_list_known_keyboards(infos, capacity, count)
    })
}

/// The models currently attached to the system.
///
/// This enumerates USB devices and may block briefly.
///
/// # Errors
///
/// Propagates any failure from the underlying enumeration.
pub fn list_available_keyboards() -> Result<Vec<KeyboardInfo>> {
    // SAFETY: forwards a caller-owned buffer with an honest capacity.
    list_with(|infos, capacity, count| unsafe {
        sys::keylt_list_available_keyboards(infos, capacity, count)
    })
}

/// Runs the ABI's probe-then-fill enumeration protocol.
///
/// A probing call with a zero capacity reports `KEYLT_ERR_BUFFER_TOO_SMALL`
/// whenever there is anything at all to report -- a zero-capacity buffer cannot
/// hold it -- and only returns `KEYLT_OK` when the result is empty. Either way
/// `out_count` receives the total, which is what sizes the real buffer.
///
/// The fill pass can itself come up short if a device is plugged in between the
/// two calls, so it is retried a bounded number of times rather than looping
/// forever against hardware that keeps changing.
fn list_with(
    mut enumerate: impl FnMut(*mut sys::keylt_keyboard_info, usize, *mut usize) -> sys::keylt_status,
) -> Result<Vec<KeyboardInfo>> {
    /// Enough for a device to settle; a caller seeing this has bigger problems.
    const MAX_ATTEMPTS: usize = 4;

    let mut count = 0usize;
    let status = enumerate(std::ptr::null_mut(), 0, &mut count);
    if status != sys::KEYLT_OK && status != sys::KEYLT_ERR_BUFFER_TOO_SMALL {
        return Err(Error::from_raw(status));
    }

    for _ in 0..MAX_ATTEMPTS {
        if count == 0 {
            return Ok(Vec::new());
        }

        let mut raw = vec![sys::keylt_keyboard_info::default(); count];
        let mut total = 0usize;
        let status = enumerate(raw.as_mut_ptr(), raw.len(), &mut total);

        if status == sys::KEYLT_ERR_BUFFER_TOO_SMALL {
            // More devices appeared than the buffer was sized for; `total` is
            // the new requirement.
            count = total;
            continue;
        }
        check(status)?;

        // Fewer devices than expected is fine: the tail is simply untouched.
        raw.truncate(total.min(count));
        return raw.iter().map(KeyboardInfo::from_raw).collect();
    }

    Err(Error::local(
        Status::Io,
        "the set of attached keyboards kept changing during enumeration",
    ))
}

/// The library's version string, such as `"0.2.0"`.
#[must_use]
pub fn version() -> &'static str {
    // SAFETY: never returns NULL and the string has static storage duration.
    let ptr = unsafe { sys::keylt_version_string() };
    static_str(ptr).unwrap_or("0.0.0-unknown")
}

/// The library's ABI version as `(major, minor, patch)`.
#[must_use]
pub fn abi_version() -> (u16, u8, u8) {
    // SAFETY: takes no arguments and never fails.
    let raw = unsafe { sys::keylt_abi_version() };
    (
        ((raw >> 16) & 0xffff) as u16,
        ((raw >> 8) & 0xff) as u8,
        (raw & 0xff) as u8,
    )
}

/// Whether the loaded library's ABI major version matches the one these
/// bindings were compiled against.
#[must_use]
pub fn abi_is_compatible() -> bool {
    sys::abi_is_compatible()
}

/// Borrows a string the library owns for the lifetime of the process.
///
/// # Safety of the `'static` bound
///
/// Every pointer passed here comes from a documented static-storage accessor
/// (`keylt_version_string`, `keylt_status_name`, `keylt_model_key_at`,
/// `keylt_sk80_lcd_defaults`), so the data genuinely outlives any borrow.
pub(crate) fn static_str(ptr: *const c_char) -> Result<&'static str> {
    if ptr.is_null() {
        return Err(Error::local(
            Status::Internal,
            "library returned a NULL string",
        ));
    }
    // SAFETY: NUL-terminated, immutable and valid for the process lifetime.
    unsafe { CStr::from_ptr(ptr) }.to_str().map_err(|err| {
        Error::local(
            Status::Internal,
            format!("library returned a non-UTF-8 string: {err}"),
        )
    })
}

/// Reads a NUL-terminated string out of a fixed-size C array.
///
/// The ABI promises the array is always terminated within its capacity, but the
/// length is bounded here anyway so a malformed buffer cannot walk off the end.
fn decode_fixed_string(buf: &[c_char]) -> String {
    // `c_char` is signed on x86-64 and unsigned on aarch64, so reinterpret
    // rather than assuming either.
    // SAFETY: `u8` and `c_char` have the same size and alignment, and the
    // resulting slice borrows the same memory for the same lifetime.
    let bytes = unsafe { std::slice::from_raw_parts(buf.as_ptr().cast::<u8>(), buf.len()) };
    let end = bytes.iter().position(|&b| b == 0).unwrap_or(bytes.len());
    String::from_utf8_lossy(&bytes[..end]).into_owned()
}
