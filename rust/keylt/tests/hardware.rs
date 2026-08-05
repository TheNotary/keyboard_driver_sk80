//! Tests that need a keyboard plugged in.
//!
//! These are `#[ignore]`d and additionally check `KEYBOARD_ATTACHED=1`, matching
//! the convention the C++ suite uses, so a run on a machine with no hardware
//! neither fails nor silently pretends to have tested anything.
//!
//! ```text
//! KEYBOARD_ATTACHED=1 cargo test -- --ignored
//! ```

use std::time::Duration;

use keylt::{KeyState, Keyboard, Model};

/// Returns `false` and explains itself when there is nothing to test against.
fn hardware_available() -> bool {
    if std::env::var("KEYBOARD_ATTACHED").as_deref() == Ok("1") {
        return true;
    }
    eprintln!("skipping: set KEYBOARD_ATTACHED=1 to run against real hardware");
    false
}

#[test]
#[ignore = "needs a keyboard attached"]
fn a_keyboard_is_enumerated() {
    if !hardware_available() {
        return;
    }
    let attached = keylt::list_available_keyboards().unwrap();
    assert!(!attached.is_empty(), "no keyboard was detected");
}

#[test]
#[ignore = "needs a keyboard attached"]
fn keys_can_be_lit_and_darkened() {
    if !hardware_available() {
        return;
    }

    let mut keyboard = Keyboard::open(Model::Sk80).unwrap();
    keyboard.connect().expect("no SK80 attached");
    assert!(keyboard.is_connected().unwrap());

    keyboard
        .set_active_key_names(&["esc", "f1", "f12"])
        .unwrap();
    assert_eq!(keyboard.active_key_count().unwrap(), 3);

    keyboard.turn_on().unwrap();
    std::thread::sleep(Duration::from_millis(250));
    keyboard.turn_off().unwrap();
}

#[test]
#[ignore = "needs a keyboard attached"]
fn captured_packets_match_the_advertised_geometry() {
    if !hardware_available() {
        return;
    }

    let mut keyboard = Keyboard::open(Model::Sk80).unwrap();
    keyboard.connect().expect("no SK80 attached");
    keyboard.set_active_key_names(&["esc"]).unwrap();

    let expected = keyboard.info().unwrap().capture_buffer_len();
    let packets = keyboard.set_keys_capture_to_vec(KeyState::On).unwrap();
    assert_eq!(packets.len(), expected);

    // A buffer one byte short must be refused without sending anything.
    let mut short = vec![0u8; expected - 1];
    let err = keyboard
        .set_keys_capture(KeyState::Off, &mut short)
        .expect_err("the buffer is too small");
    assert_eq!(err.status(), keylt::Status::BufferTooSmall);

    keyboard.turn_off().unwrap();
}
