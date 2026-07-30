//! Drives the LCD through a USB transport written in Rust.
//!
//! Passing your own [`keylt::UsbIo`] replaces the library's HID backend
//! entirely, which is how you mock the device, log the protocol, or plug in a
//! different HID stack. This example needs no hardware.
//!
//! ```text
//! cargo run --example custom_usb_io
//! ```

use std::io;
use std::time::Duration;

use keylt::{Lcd, UsbIo};

/// A transport that logs every call and pretends everything worked.
#[derive(Default)]
struct Logger {
    bytes_written: usize,
}

impl UsbIo for Logger {
    fn open(&mut self, vid: u16, pid: u16, control: &str, data: &str) -> bool {
        println!("open({vid:04x}:{pid:04x}, control={control:?}, data={data:?})");
        true
    }

    fn send_feature_report(&mut self, data: &[u8]) -> io::Result<usize> {
        println!(
            "send_feature_report({} bytes): {:02x?}",
            data.len(),
            first(data)
        );
        Ok(data.len())
    }

    fn get_feature_report(&mut self, buffer: &mut [u8]) -> io::Result<usize> {
        println!("get_feature_report({} bytes)", buffer.len());
        buffer.fill(0);
        Ok(buffer.len())
    }

    fn write_data(&mut self, data: &[u8]) -> io::Result<usize> {
        self.bytes_written += data.len();
        println!(
            "write_data({} bytes, {} total)",
            data.len(),
            self.bytes_written
        );
        Ok(data.len())
    }

    fn read_data(&mut self, buffer: &mut [u8], timeout: Duration) -> io::Result<usize> {
        println!("read_data({} bytes, timeout {timeout:?})", buffer.len());
        buffer.fill(0);
        Ok(buffer.len())
    }

    fn close(&mut self) {
        println!("close() after {} bytes", self.bytes_written);
    }
}

/// Keeps the log readable when a packet is large.
fn first(data: &[u8]) -> &[u8] {
    &data[..data.len().min(8)]
}

fn main() -> Result<(), keylt::Error> {
    let mut lcd = Lcd::open_sk80_with(Logger::default())?;

    // No USB enumeration happens: `connect` goes straight to `Logger::open`.
    lcd.connect()?;
    println!("connected: {}", lcd.is_connected()?);

    // An upload only runs when a real image is supplied, since the library has
    // to decode the file before any traffic is generated.
    if let Some(path) = std::env::args().nth(1) {
        lcd.upload_image(&path)?;
    } else {
        println!("pass an image path to see the upload traffic");
    }

    // Dropping the handle closes it, which reaches `Logger::close`.
    drop(lcd);
    Ok(())
}
