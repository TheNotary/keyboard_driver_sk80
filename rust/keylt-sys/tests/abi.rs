//! Checks that the declarations in this crate agree with the loaded library.
//!
//! These run against whichever binding source is active -- the checked-in
//! declarations or the `bindgen` feature -- which is what keeps the hand-written
//! copy honest.

use std::ffi::CStr;
use std::mem::{align_of, offset_of, size_of};

use keylt_sys as sys;

#[test]
fn keyboard_info_layout_matches_the_header() {
    // Every field is fixed-width and the struct carries explicit padding, so
    // the layout is fully determined: 12 bytes of scalars, 25 of name, 3 of tail.
    assert_eq!(size_of::<sys::keylt_keyboard_info>(), 40);
    assert_eq!(align_of::<sys::keylt_keyboard_info>(), 4);

    assert_eq!(offset_of!(sys::keylt_keyboard_info, model), 0);
    assert_eq!(offset_of!(sys::keylt_keyboard_info, vid), 4);
    assert_eq!(offset_of!(sys::keylt_keyboard_info, pid), 6);
    assert_eq!(offset_of!(sys::keylt_keyboard_info, max_key_id), 8);
    assert_eq!(offset_of!(sys::keylt_keyboard_info, message_length), 9);
    assert_eq!(
        offset_of!(sys::keylt_keyboard_info, bulk_led_value_messages_count),
        10
    );
    assert_eq!(offset_of!(sys::keylt_keyboard_info, reserved), 11);
    assert_eq!(offset_of!(sys::keylt_keyboard_info, display_name), 12);
    assert_eq!(offset_of!(sys::keylt_keyboard_info, padding), 37);
}

#[test]
fn display_name_capacity_matches_the_field() {
    let info = sys::keylt_keyboard_info::default();
    assert_eq!(
        info.display_name.len(),
        sys::KEYLT_DISPLAY_NAME_CAPACITY as usize
    );
}

#[test]
fn usb_io_vtable_is_pointer_sized_throughout() {
    // Seven pointers: one user_data and six callbacks. If the library ever
    // grows the vtable, this is the first thing to fail.
    assert_eq!(
        size_of::<sys::keylt_usb_io>(),
        7 * size_of::<*const std::ffi::c_void>()
    );
    let vtable = sys::keylt_usb_io::default();
    assert!(vtable.user_data.is_null());
    assert!(vtable.open.is_none());
    assert!(vtable.close.is_none());
}

#[test]
fn loaded_library_has_a_compatible_abi() {
    assert!(
        sys::abi_is_compatible(),
        "loaded library reports ABI {:#x}, these bindings expect major {}",
        unsafe { sys::keylt_abi_version() },
        sys::KEYLT_ABI_VERSION >> 16
    );
}

#[test]
fn abi_version_constant_is_encoded_as_documented() {
    assert_eq!(sys::KEYLT_ABI_VERSION, 0x0001_0000);
    assert_eq!(unsafe { sys::keylt_abi_version() } >> 16, 1);
}

#[test]
fn version_string_is_populated() {
    let ptr = unsafe { sys::keylt_version_string() };
    assert!(!ptr.is_null());
    let version = unsafe { CStr::from_ptr(ptr) }.to_str().unwrap();
    assert!(!version.is_empty());
}

#[test]
fn status_names_round_trip() {
    // Confirms the constants in this crate line up with the library's own table
    // rather than having drifted.
    for (status, expected) in [
        (sys::KEYLT_OK, "KEYLT_OK"),
        (
            sys::KEYLT_ERR_INVALID_ARGUMENT,
            "KEYLT_ERR_INVALID_ARGUMENT",
        ),
        (
            sys::KEYLT_ERR_DEVICE_NOT_FOUND,
            "KEYLT_ERR_DEVICE_NOT_FOUND",
        ),
        (sys::KEYLT_ERR_OUT_OF_RANGE, "KEYLT_ERR_OUT_OF_RANGE"),
        (sys::KEYLT_ERR_UNSUPPORTED, "KEYLT_ERR_UNSUPPORTED"),
        (sys::KEYLT_ERR_IO, "KEYLT_ERR_IO"),
        (sys::KEYLT_ERR_OUT_OF_MEMORY, "KEYLT_ERR_OUT_OF_MEMORY"),
        (
            sys::KEYLT_ERR_BUFFER_TOO_SMALL,
            "KEYLT_ERR_BUFFER_TOO_SMALL",
        ),
        (sys::KEYLT_ERR_INTERNAL, "KEYLT_ERR_INTERNAL"),
    ] {
        let name = unsafe { CStr::from_ptr(sys::keylt_status_name(status)) }
            .to_str()
            .unwrap();
        assert_eq!(name, expected, "status {status}");
    }
}

#[test]
fn enumeration_reports_a_count_without_a_buffer() {
    // The probe half of the caller-allocated buffer protocol. A zero capacity
    // cannot hold a non-empty result, so the call reports BUFFER_TOO_SMALL and
    // still writes the total that a real buffer would need.
    let mut count = 0usize;
    let status = unsafe { sys::keylt_list_known_keyboards(std::ptr::null_mut(), 0, &mut count) };
    assert_eq!(status, sys::KEYLT_ERR_BUFFER_TOO_SMALL);
    assert!(count > 0, "the library should know at least one model");

    // The fill half: an exactly-sized buffer succeeds.
    let mut infos = vec![sys::keylt_keyboard_info::default(); count];
    let mut total = 0usize;
    let status =
        unsafe { sys::keylt_list_known_keyboards(infos.as_mut_ptr(), infos.len(), &mut total) };
    assert_eq!(status, sys::KEYLT_OK);
    assert_eq!(total, count);
    assert!(infos.iter().all(|info| info.display_name[0] != 0));
}
