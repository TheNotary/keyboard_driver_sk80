//! Tests that need no keyboard attached.

use keylt::{Key, KeyState, Keyboard, Model, Rgb, Status};

#[test]
fn abi_is_compatible_with_the_loaded_library() {
    assert!(keylt::abi_is_compatible());
    assert_eq!(keylt::abi_version().0, 1);
}

#[test]
fn version_is_reported() {
    let version = keylt::version();
    assert!(!version.is_empty());
    assert!(
        version.chars().next().is_some_and(|c| c.is_ascii_digit()),
        "unexpected version string {version:?}"
    );
}

#[test]
fn known_keyboards_include_the_implemented_models() {
    let known = keylt::list_known_keyboards().expect("enumeration should succeed");
    assert!(!known.is_empty());

    let models: Vec<Model> = known.iter().map(|info| info.model).collect();
    assert!(
        models.contains(&Model::Sk80),
        "SK80 missing from {models:?}"
    );
    assert!(
        models.contains(&Model::Rk84),
        "RK84 missing from {models:?}"
    );

    for info in &known {
        assert!(!info.display_name.is_empty());
        assert!(info.max_key_id > 0);
    }
}

#[test]
fn available_keyboards_enumerates_without_hardware() {
    // With nothing attached this is empty, but it must not fail: the probe and
    // fill passes both have to survive a zero count.
    let available = keylt::list_available_keyboards().expect("enumeration should succeed");
    for info in &available {
        assert!(!info.display_name.is_empty());
    }
}

#[test]
fn model_info_describes_packet_geometry() {
    let info = Model::Sk80.info().expect("SK80 is implemented");
    assert_eq!(info.model, Model::Sk80);
    assert!(info.message_length > 0);
    assert!(info.bulk_led_value_messages_count > 0);
    assert_eq!(
        info.capture_buffer_len(),
        usize::from(info.message_length) * usize::from(info.bulk_led_value_messages_count)
    );
}

#[test]
fn unimplemented_model_is_reported_as_unsupported() {
    let err = Model::Mk84.info().expect_err("MK84 is not implemented");
    assert_eq!(err.status(), Status::Unsupported);
    assert_eq!(err.status().name(), "KEYLT_ERR_UNSUPPORTED");
}

#[test]
fn key_table_is_indexable_and_stable() {
    let count = Model::Sk80.key_count().expect("SK80 has a key table");
    assert!(count > 0);

    let keys = Model::Sk80.keys().expect("SK80 has a key table");
    assert_eq!(keys.len(), count);

    // Indices are documented as stable for the process lifetime.
    let again = Model::Sk80.keys().expect("SK80 has a key table");
    assert_eq!(keys, again);

    let max_key_id = Model::Sk80.info().unwrap().max_key_id;
    for Key { name, id } in &keys {
        assert!(!name.is_empty());
        assert!(*id <= max_key_id, "{name} has out-of-range id {id}");
    }
}

#[test]
fn key_ids_resolve_from_names() {
    let keys = Model::Sk80.keys().unwrap();
    for Key { name, id } in keys.iter().take(16) {
        assert_eq!(Model::Sk80.key_id_for_name(name).unwrap(), *id);
    }
}

#[test]
fn unknown_key_name_is_rejected() {
    let err = Model::Sk80
        .key_id_for_name("no-such-key")
        .expect_err("the model does not define that name");
    assert_eq!(err.status(), Status::InvalidArgument);
}

#[test]
fn interior_nul_in_a_key_name_is_rejected_without_calling_the_library() {
    let err = Model::Sk80
        .key_id_for_name("es\0c")
        .expect_err("interior NUL cannot be passed to C");
    assert_eq!(err.status(), Status::InvalidArgument);
    assert!(err.message().contains("NUL"), "{}", err.message());
}

#[test]
fn key_index_past_the_end_is_out_of_range() {
    let count = Model::Sk80.key_count().unwrap();
    let err = Model::Sk80
        .key_at(count)
        .expect_err("index is past the end");
    assert_eq!(err.status(), Status::OutOfRange);
}

#[test]
fn opening_a_handle_does_not_require_hardware() {
    let keyboard = Keyboard::open(Model::Sk80).expect("opening must not touch the device");
    assert_eq!(keyboard.model(), Model::Sk80);
    assert!(!keyboard.is_connected().unwrap());
    assert_eq!(keyboard.active_key_count().unwrap(), 0);
}

#[test]
fn opening_an_unimplemented_model_fails() {
    let err = Keyboard::open(Model::Mk84).expect_err("MK84 is not implemented");
    assert_eq!(err.status(), Status::Unsupported);
}

#[test]
fn active_keys_can_be_selected_and_cleared_offline() {
    let mut keyboard = Keyboard::open(Model::Sk80).unwrap();

    keyboard.set_active_key_names(&["esc", "f1", "f2"]).unwrap();
    assert_eq!(keyboard.active_key_count().unwrap(), 3);

    keyboard.set_active_key_ids(&[1, 2, 3, 4]).unwrap();
    assert_eq!(keyboard.active_key_count().unwrap(), 4);

    keyboard.clear_active_keys().unwrap();
    assert_eq!(keyboard.active_key_count().unwrap(), 0);
}

#[test]
fn out_of_range_key_id_leaves_the_selection_intact() {
    let mut keyboard = Keyboard::open(Model::Sk80).unwrap();
    keyboard.set_active_key_names(&["esc"]).unwrap();

    let max_key_id = keyboard.info().unwrap().max_key_id;
    let err = keyboard
        .set_active_key_ids(&[1, max_key_id.saturating_add(1)])
        .expect_err("ids above max_key_id are rejected");
    assert_eq!(err.status(), Status::OutOfRange);

    // Validation is documented as all-or-nothing.
    assert_eq!(keyboard.active_key_count().unwrap(), 1);
}

#[test]
fn unknown_key_name_leaves_the_selection_intact() {
    let mut keyboard = Keyboard::open(Model::Sk80).unwrap();
    keyboard.set_active_key_names(&["esc", "f1"]).unwrap();

    let err = keyboard
        .set_active_key_names(&["esc", "no-such-key"])
        .expect_err("names are resolved up front");
    assert_eq!(err.status(), Status::InvalidArgument);
    assert_eq!(keyboard.active_key_count().unwrap(), 2);
}

#[test]
fn set_active_key_names_accepts_owned_strings() {
    // The generic bound should cover both `&str` and `String` without a cast.
    let mut keyboard = Keyboard::open(Model::Sk80).unwrap();
    let names: Vec<String> = vec!["esc".to_owned(), "f1".to_owned()];
    keyboard.set_active_key_names(&names).unwrap();
    assert_eq!(keyboard.active_key_count().unwrap(), 2);
}

#[test]
fn driving_a_disconnected_keyboard_fails_cleanly() {
    let mut keyboard = Keyboard::open(Model::Sk80).unwrap();
    keyboard.set_active_key_names(&["esc"]).unwrap();

    let err = keyboard
        .set_keys(KeyState::On)
        .expect_err("nothing is connected");
    assert_eq!(err.status(), Status::DeviceNotFound);
}

#[test]
fn key_state_maps_to_the_c_abi_ordering() {
    // The C ABI uses OFF = 0, ON = 1, which is the opposite of the library's
    // internal enum. A regression here would silently invert every call.
    assert_eq!(KeyState::Off.as_raw(), keylt::sys::KEYLT_KEY_OFF);
    assert_eq!(KeyState::On.as_raw(), keylt::sys::KEYLT_KEY_ON);
    assert_eq!(KeyState::from(true), KeyState::On);
    assert_eq!(KeyState::from(false), KeyState::Off);
}

#[test]
fn models_round_trip_through_their_raw_values() {
    for model in Model::ALL {
        let raw = i32::try_from(model.as_raw()).unwrap();
        assert_eq!(Model::from_raw(raw), Some(model));
    }
    assert_eq!(Model::from_raw(-1), None);
    assert_eq!(Model::from_raw(9999), None);
}

#[test]
fn rgb_converts_from_tuples_and_arrays() {
    assert_eq!(Rgb::from((1, 2, 3)), Rgb::new(1, 2, 3));
    assert_eq!(Rgb::from([1, 2, 3]), Rgb::new(1, 2, 3));
}

#[test]
fn print_messages_rejects_a_buffer_that_is_too_short() {
    let err = keylt::print_messages(&[0u8; 8], 2, 8).expect_err("8 bytes cannot hold 2 x 8");
    assert_eq!(err.status(), Status::InvalidArgument);

    // Overflowing the geometry must be caught rather than wrapping.
    let err = keylt::print_messages(&[0u8; 8], usize::MAX, 2).expect_err("geometry overflows");
    assert_eq!(err.status(), Status::InvalidArgument);
}

#[test]
fn errors_display_the_status_name() {
    let err = Model::Mk84.info().unwrap_err();
    assert!(
        err.to_string().starts_with("KEYLT_ERR_UNSUPPORTED"),
        "{err}"
    );
}

#[test]
fn keyboard_handles_move_between_threads() {
    // `Keyboard` is `Send` but not `Sync`; this pins the `Send` half.
    let keyboard = Keyboard::open(Model::Sk80).unwrap();
    let model = std::thread::spawn(move || keyboard.model()).join().unwrap();
    assert_eq!(model, Model::Sk80);
}
