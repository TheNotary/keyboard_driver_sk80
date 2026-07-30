//! Drives an [`Lcd`] through a Rust-supplied USB transport, so the whole
//! callback path is exercised without any hardware.

use std::io;
use std::sync::{Arc, Mutex};
use std::time::Duration;

use blink::{Lcd, UsbIo};

/// One call the library made into the transport.
#[derive(Debug, Clone, PartialEq, Eq)]
enum Call {
    Open { vid: u16, pid: u16 },
    SendFeatureReport(Vec<u8>),
    GetFeatureReport(usize),
    WriteData(Vec<u8>),
    ReadData(usize),
    Close,
}

/// A transport that records what it was asked to do and always succeeds.
#[derive(Default)]
struct Recorder {
    calls: Arc<Mutex<Vec<Call>>>,
}

impl Recorder {
    fn with_log(calls: Arc<Mutex<Vec<Call>>>) -> Self {
        Self { calls }
    }

    fn record(&mut self, call: Call) {
        self.calls.lock().unwrap().push(call);
    }
}

impl UsbIo for Recorder {
    fn open(&mut self, vid: u16, pid: u16, _control: &str, _data: &str) -> bool {
        self.record(Call::Open { vid, pid });
        true
    }

    fn send_feature_report(&mut self, data: &[u8]) -> io::Result<usize> {
        self.record(Call::SendFeatureReport(data.to_vec()));
        Ok(data.len())
    }

    fn get_feature_report(&mut self, buffer: &mut [u8]) -> io::Result<usize> {
        self.record(Call::GetFeatureReport(buffer.len()));
        buffer.fill(0);
        Ok(buffer.len())
    }

    fn write_data(&mut self, data: &[u8]) -> io::Result<usize> {
        self.record(Call::WriteData(data.to_vec()));
        Ok(data.len())
    }

    fn read_data(&mut self, buffer: &mut [u8], _timeout: Duration) -> io::Result<usize> {
        self.record(Call::ReadData(buffer.len()));
        buffer.fill(0);
        Ok(buffer.len())
    }

    fn close(&mut self) {
        self.record(Call::Close);
    }
}

/// A transport whose `open` always fails.
struct Unavailable;

impl UsbIo for Unavailable {
    fn open(&mut self, _vid: u16, _pid: u16, _control: &str, _data: &str) -> bool {
        false
    }
}

/// A transport that panics, to prove the trampolines contain the unwind.
struct Exploding;

impl UsbIo for Exploding {
    fn open(&mut self, _vid: u16, _pid: u16, _control: &str, _data: &str) -> bool {
        panic!("this must not unwind across the FFI boundary");
    }
}

#[test]
fn defaults_describe_the_sk80_lcd() {
    let defaults = blink::sk80_lcd_defaults().expect("defaults are always available");
    assert_ne!(defaults.vid, 0);
    assert_ne!(defaults.pid, 0);
    assert!(!defaults.control_interface.is_empty());

    // The data interface is deliberately empty on Windows: that HID path has
    // never been captured, and the Windows LCD backend is still a stub, so the
    // constant exists only so the shared code compiles. See the TODO alongside
    // lcd_data_device_path in include/main/keyboards/sk80/constants_sk80.h.
    // Asserting both directions means whichever platform changes first trips
    // this test rather than silently diverging.
    if cfg!(windows) {
        assert!(
            defaults.data_interface.is_empty(),
            "Windows now reports an LCD data interface ({:?}); the backend and \
             this test both need revisiting",
            defaults.data_interface
        );
    } else {
        assert!(!defaults.data_interface.is_empty());
    }
}

#[test]
fn a_rust_transport_receives_the_open_call() {
    let calls = Arc::new(Mutex::new(Vec::new()));
    let defaults = blink::sk80_lcd_defaults().unwrap();

    let mut lcd = Lcd::open_sk80_with(Recorder::with_log(Arc::clone(&calls)))
        .expect("opening a handle must not touch hardware");
    assert!(!lcd.is_connected().unwrap());

    lcd.connect().expect("the recorder accepts every open");
    assert!(lcd.is_connected().unwrap());

    let recorded = calls.lock().unwrap().clone();
    assert_eq!(
        recorded.first(),
        Some(&Call::Open {
            vid: defaults.vid,
            pid: defaults.pid
        })
    );

    // Closing the handle must reach the transport, and the transport must
    // outlive the handle for that to be sound.
    drop(lcd);
    assert_eq!(calls.lock().unwrap().last(), Some(&Call::Close));
}

#[test]
fn a_transport_that_cannot_open_is_reported_as_device_not_found() {
    let mut lcd = Lcd::open_sk80_with(Unavailable).unwrap();
    let err = lcd.connect().expect_err("open returned false");
    assert_eq!(err.status(), blink::Status::DeviceNotFound);
    assert!(!lcd.is_connected().unwrap());
}

#[test]
fn a_panicking_transport_does_not_unwind_into_c() {
    // If the trampoline let this escape, the process would abort instead of
    // returning an error.
    let mut lcd = Lcd::open_sk80_with(Exploding).unwrap();
    let err = lcd
        .connect()
        .expect_err("the panic is converted to a failure");
    assert_eq!(err.status(), blink::Status::DeviceNotFound);
}

#[test]
fn the_default_transport_needs_no_backend() {
    // Opening against the library's built-in USB backend must still work when
    // no device is attached.
    let lcd = Lcd::open_sk80().expect("opening a handle must not touch hardware");
    assert!(!lcd.is_connected().unwrap());
}

#[test]
fn interface_selectors_with_interior_nuls_are_rejected() {
    let err = Lcd::open(0x1234, 0x5678, "cont\0rol", "data")
        .expect_err("interior NUL cannot be passed to C");
    assert_eq!(err.status(), blink::Status::InvalidArgument);
}

#[test]
fn a_non_utf8_image_path_is_rejected() {
    #[cfg(unix)]
    {
        use std::ffi::OsStr;
        use std::os::unix::ffi::OsStrExt;
        use std::path::Path;

        let mut lcd = Lcd::open_sk80_with(Recorder::default()).unwrap();
        let path = Path::new(OsStr::from_bytes(b"/tmp/\xff\xfe.gif"));
        let err = lcd
            .upload_image(path)
            .expect_err("the C ABI takes a const char*");
        assert_eq!(err.status(), blink::Status::InvalidArgument);
        assert!(err.message().contains("UTF-8"), "{}", err.message());
    }
}
