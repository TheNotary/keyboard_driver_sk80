# XVX S-K80 Keyboard Driver for LED Control

[![CI](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/ci.yaml/badge.svg)](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/ci.yaml)
[![Release](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/release.yaml/badge.svg)](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/release.yaml)

> Warning: One keeb was lost during the development of this driver.  XVX S-K80 and company (Womier seems to be an alternate name) does not ship with a very robust firmware and has open vulnerabilities that allow any non-root program on your computer to permenently destroy the device.  If your computer emmits the wrong packets to the keyboard, it could actually brick the device.  While this repo should be safe for S-K80's produced between 2023 and 2024, knowing the firmware issues associated with this product, I recommend using this repo (and their official drivers for that matter) only if you're willing to accept the risk of losing the device.

This project builds a userspace driver (`blink.dll` on Windows, `libblink.so` on Linux) that can control the LEDs on an XVX S-K80 mechanical keyboard.  It happens to also support the RK84 by Royal Kludge.


#### Building on Windows ("Developer Command Prompt")

To conduct the build and execute the tests, at the root of the repo, run:

```
setup.bat
```

#### Building on Linux (Arch)

```
sudo pacman -S base-devel cmake ninja hidapi pkgconf
./setup.sh
```

Non-root access to the keyboard requires a udev rule.  Copy the included rules file, reload, and ensure your user is in the `wheel` group:

```
sudo cp 99-keyboard-leds.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

You may need to replug the keyboard or log out and back in for the new permissions to take effect.

For more notes about vcpkg, CMake, and building see [here](docs/env_setup_and_building.md).

## Using blink in your own project

Every tagged release publishes a prebuilt package for `x86_64`/`aarch64` on both Linux and Windows.  Each archive contains headers, a shared and a static library, and a CMake package config.  The Linux archives also carry a pkg-config `.pc` file.

```cmake
find_package(blink CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE blink::blink)         # shared
target_link_libraries(my_app PRIVATE blink::blink_static)  # or static
```

```c
#include <blink/blink.h>
```

Or, without CMake (Linux):

```bash
cc my_app.c $(pkg-config --cflags --libs blink)
```

Download and extract the archive for your platform from the [releases page](https://github.com/TheNotary/keyboard_driver_sk80/releases), then point CMake at it:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/extracted/blink
```

Alternatively, use the `blink-vcpkg-port.tar.gz` asset as a vcpkg overlay port:

```bash
tar xzf blink-vcpkg-port.tar.gz -C ./overlay-ports
vcpkg install blink --overlay-ports=./overlay-ports
```

> `blink.h` is the complete public interface and is valid C99: it includes no C++ headers, declares no C++ types, and every symbol has C language linkage.  CI proves both halves of that — `nm -D` checks that `libblink.so` exports nothing but `blink_*`, and a C99 consumer is compiled against the installed header on every leg.  A prebuilt **shared** library therefore links against anything with a C FFI, whatever toolchain built it.
>
> The **static** archive is the exception.  It is C++ underneath, so it still requires a compatible toolchain (matching MSVC toolset and CRT on Windows, matching libstdc++ ABI on Linux), and a C program linking it needs the C++ runtime — in CMake, `set_target_properties(my_app PROPERTIES LINKER_LANGUAGE CXX)`.  When in doubt, link the shared library or build from source via the vcpkg port.

## Using blink from Rust

The [`rust/`](rust) directory holds a Cargo workspace with two crates: `blink-sys` for the raw FFI declarations, and `blink` for the safe wrapper.

```rust
use blink::{KeyState, Keyboard, Model};

let mut keyboard = Keyboard::open(Model::Sk80)?;
keyboard.connect()?;
keyboard.set_active_key_names(&["esc", "f1", "f2"])?;
keyboard.set_keys(KeyState::On)?;
```

With no setup at all, `cargo test` builds the C library from source through CMake.  To link a package you have already installed or extracted:

```bash
cd rust
BLINK_LIB_DIR=/path/to/blink/lib BLINK_INCLUDE_DIR=/path/to/blink/include \
  cargo build --no-default-features --features static
```

pkg-config works too, via the `blink.pc` in the install tree.  See [rust/README.md](rust/README.md) for the discovery modes, feature flags, and the safety contract the wrapper enforces.

