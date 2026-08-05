//! Lists the keyboard models the library knows about and the ones attached.
//!
//! ```text
//! cargo run --example list_keyboards
//! ```

fn main() -> Result<(), keylt::Error> {
    println!(
        "keylt {} (ABI {}.{}.{})",
        keylt::version(),
        keylt::abi_version().0,
        keylt::abi_version().1,
        keylt::abi_version().2
    );

    println!("\nKnown models:");
    for info in keylt::list_known_keyboards()? {
        println!(
            "  {:<8} {:04x}:{:04x}  max key id {:>3}  packets {}x{} bytes",
            info.display_name,
            info.vid,
            info.pid,
            info.max_key_id,
            info.bulk_led_value_messages_count,
            info.message_length,
        );

        // MK84 is listed but unimplemented, so asking for its key table fails.
        match info.model.key_count() {
            Ok(count) => println!("           {count} named keys"),
            Err(err) => println!("           key table unavailable: {err}"),
        }
    }

    println!("\nAttached now:");
    let attached = keylt::list_available_keyboards()?;
    if attached.is_empty() {
        println!("  (none)");
    }
    for info in attached {
        println!(
            "  {:<8} {:04x}:{:04x}",
            info.display_name, info.vid, info.pid
        );
    }

    Ok(())
}
