//! Uploads a still image or animated GIF to the SK80's LCD.
//!
//! Needs a keyboard attached. The transfer blocks for a long time and cannot be
//! cancelled.
//!
//! ```text
//! cargo run --example upload_lcd_image -- splash.gif
//! ```

use std::process::ExitCode;

fn main() -> ExitCode {
    let Some(path) = std::env::args().nth(1) else {
        eprintln!("usage: upload_lcd_image <image path>");
        return ExitCode::FAILURE;
    };

    match upload(&path) {
        Ok(()) => ExitCode::SUCCESS,
        Err(err) => {
            eprintln!("upload failed: {err}");
            ExitCode::FAILURE
        }
    }
}

fn upload(path: &str) -> Result<(), blink::Error> {
    let defaults = blink::sk80_lcd_defaults()?;
    println!(
        "SK80 LCD is {:04x}:{:04x} on interfaces {:?} / {:?}",
        defaults.vid, defaults.pid, defaults.control_interface, defaults.data_interface
    );

    let mut lcd = blink::Lcd::open_sk80()?;
    lcd.connect()?;

    println!("uploading {path}, this takes a while and reports no progress...");
    lcd.upload_image(path)?;
    println!("done");
    Ok(())
}
