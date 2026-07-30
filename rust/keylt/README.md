# keylt

Safe Rust bindings for controlling the LEDs and LCD of RGB mechanical keyboards
(Sharkoon SKILLER SGK80, Royal Kludge RK84).

```rust
use keylt::{KeyState, Keyboard, Model};

let mut keyboard = Keyboard::open(Model::Sk80)?;
keyboard.connect()?;
keyboard.set_active_key_names(&["esc", "f1", "f2"])?;
keyboard.set_keys(KeyState::On)?;
# Ok::<(), keylt::Error>(())
```

Opaque handles become RAII types, status codes become `Result`s, and the C
ABI's caller-allocated buffer protocols are hidden behind ordinary slices and
`Vec`s. Supply your own USB transport by implementing `UsbIo`.

The native library is located by [`keylt-sys`](https://crates.io/crates/keylt-sys),
which by default builds it from source with CMake. See
[`rust/README.md`](https://github.com/TheNotary/keyboard_driver_sk80/blob/main/rust/README.md)
for the other discovery modes and the feature flags.
