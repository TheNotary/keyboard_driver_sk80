# Rust bindings for blink

Two crates, in the usual `-sys` plus wrapper arrangement:

| Crate | What it is |
| --- | --- |
| [`blink-sys`](blink-sys) | Raw FFI declarations for the C ABI in `include/main/blink.h`, plus the build script that finds and links the native library. |
| [`blink`](blink) | The safe wrapper: RAII handles, `Result`s, slices instead of caller-allocated buffers. This is the one to use. |

Only the public C ABI is bound. The internal C++ headers that
`BLINK_INSTALL_CXX_HEADERS` installs are deliberately out of scope.

## Quick start

```rust
use blink::{KeyState, Keyboard, Model};

let mut keyboard = Keyboard::open(Model::Sk80)?;
keyboard.connect()?;
keyboard.set_active_key_names(&["esc", "f1", "f2"])?;
keyboard.set_keys(KeyState::On)?;
# Ok::<(), blink::Error>(())
```

```console
$ cargo run --example list_keyboards
blink 0.2.0 (ABI 1.0.0)

Known models:
  RK84     258a:00c0  max key id  96  packets 3x65 bytes
           84 named keys
  SK80     05ac:024f  max key id 121  packets 9x65 bytes
           81 named keys

Attached now:
  SK80     05ac:024f
```

## Finding the native library

`blink-sys` tries three things, in order, and takes the first that works.

### 1. An explicit prefix

```console
$ cmake --preset ci && cmake --build --preset ci
$ cmake --install build/ci --prefix /tmp/blink-prefix

$ cd rust
$ BLINK_LIB_DIR=/tmp/blink-prefix/lib \
  BLINK_INCLUDE_DIR=/tmp/blink-prefix/include \
  cargo test --no-default-features
```

`BLINK_INCLUDE_DIR` must contain `blink/blink.h`. Both variables are required
together; setting one alone is ignored.

### 2. pkg-config

The CMake project installs `lib/pkgconfig/blink.pc`, which locates its own
prefix relative to itself and so survives being moved:

```console
$ PKG_CONFIG_PATH=/tmp/blink-prefix/lib/pkgconfig \
  cargo test --no-default-features
```

### 3. Building from source (the default)

With no environment set up at all, the `vendored` feature configures and builds
the CMake project at the repository root and links the resulting static archive:

```console
$ cd rust && cargo test
```

This needs CMake and the vcpkg checkout bundled with the repository, and it is
much slower than the other two. It passes `VCPKG_MANIFEST_NO_DEFAULT_FEATURES`
so the build does not also drag in gtest and google-benchmark.

## Features

| Feature | Default | Effect |
| --- | --- | --- |
| `vendored` | yes | Build the C library from source with CMake. Implies static linkage. |
| `static` | no | Link the static archive when using an explicit prefix or pkg-config. |
| `bindgen` | no | Regenerate the FFI declarations from `blink.h` instead of using the checked-in ones. Needs libclang. |

### Static or shared?

Linking the **shared** library is the default when you supply a prefix, and is
what CI does. It is the more robust choice: `libblink.so` encapsulates hidapi
behind `--exclude-libs`, so nothing on your side has to know about blink's
private dependencies. The cost is that the loader has to find the library at run
time, and a build script cannot inject an rpath into the crates that depend on
it, so that part is on you:

```console
$ LD_LIBRARY_PATH=/tmp/blink-prefix/lib cargo test --no-default-features
```

Linking the **static** archive with `--features static` produces self-contained
binaries, which is why `vendored` uses it. Be aware that `blink_static` exposes
hidapi as a *public* usage requirement, so the final link line has to resolve
hidapi, libusb and libudev as well. `blink-sys` asks pkg-config for those, which
works wherever a system hidapi is installed with a complete `.pc` file — but not,
for instance, against a vcpkg-built hidapi, whose `hidapi-libusb.pc` declares no
private dependencies at all.

## Bindings

`blink-sys/src/bindings.rs` is written by hand so that the common case needs no
libclang. It is not a stale copy of bindgen output: CI regenerates the
declarations with `--features bindgen` and runs the whole suite against them, so
any drift in a signature or a struct layout fails the build. `blink-sys/tests/abi.rs`
asserts the layout of `blink_keyboard_info` and the size of the `blink_usb_io`
vtable against the loaded library either way.

## Safety notes

The wrapper enforces the parts of the C contract that Rust can express:

* `Keyboard` and `Lcd` are `Send` but not `Sync`. The library takes no locks, so
  one handle must not be driven from two threads at once.
* The thread-local last-error message is copied into `Error` before any call
  returns, because the pointer the library hands back is only valid until the
  next call on that thread.
* Every `UsbIo` callback runs inside `catch_unwind`. Unwinding into C is
  undefined behaviour, and the header forbids it outright.
* `blink_key_value` is `OFF = 0, ON = 1`, the opposite of the library's internal
  enum. `KeyState` maps it explicitly and never casts; `offline.rs` pins that
  down.

## Tests

```console
$ cargo test                       # hardware-free
$ KEYBOARD_ATTACHED=1 cargo test -- --ignored   # needs a keyboard
```

The hardware-free suite covers enumeration, the key tables, error mapping, and
the whole `UsbIo` callback path via a recording backend — including a
deliberately panicking transport, to prove the trampolines contain it.

## Examples

| Example | Hardware | What it shows |
| --- | --- | --- |
| `list_keyboards` | no | Enumeration and model introspection. |
| `custom_usb_io` | no | Driving an `Lcd` through a Rust `UsbIo` transport. |
| `blink_key` | yes | Selecting keys, capturing the packets, blinking. |
| `upload_lcd_image` | yes | Sending a still or animated GIF to the LCD. |
