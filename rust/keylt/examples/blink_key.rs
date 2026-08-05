//! Lights a set of keys, blinks them, then turns them off again.
//!
//! Needs a keyboard attached.
//!
//! ```text
//! cargo run --example blink_key -- esc f1 f2
//! ```

use std::time::Duration;

use keylt::{KeyState, Keyboard, Model};

fn main() -> Result<(), keylt::Error> {
    let names: Vec<String> = std::env::args().skip(1).collect();
    let names = if names.is_empty() {
        vec!["esc".to_owned()]
    } else {
        names
    };

    let mut keyboard = Keyboard::open(Model::Sk80)?;
    keyboard.connect()?;

    let info = keyboard.info()?;
    println!(
        "connected to {} ({:04x}:{:04x})",
        info.display_name, info.vid, info.pid
    );

    keyboard.set_active_key_names(&names)?;
    println!(
        "selected {} key(s): {}",
        keyboard.active_key_count()?,
        names.join(", ")
    );

    // `set_keys_capture_to_vec` sends exactly what `set_keys` would and hands
    // back the packets, which is handy when comparing against a USB capture.
    let packets = keyboard.set_keys_capture_to_vec(KeyState::On)?;
    println!("sent {} bytes to light the selection", packets.len());
    keylt::print_messages(
        &packets,
        usize::from(info.bulk_led_value_messages_count),
        usize::from(info.message_length),
    )?;

    // This blocks for roughly count * interval * 7.
    keyboard.blink(3, Duration::from_millis(120))?;

    keyboard.set_keys(KeyState::Off)?;
    Ok(())
}
