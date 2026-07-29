# XVX S-K80 Keyboard Driver for LED Control

[![CI](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/ci.yaml/badge.svg)](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/ci.yaml)
[![Release](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/release.yaml/badge.svg)](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/release.yaml)

> Warning: One keeb was lost during the development of this driver.  XVX S-K80 and company (Womier seems to be an alternate name) does not ship with a very robust firmware and has open vulnerabilities that allow any non-root program on your computer to permenently destroy the device.  If your computer emmits the wrong packets to the keyboard, it could actually brick the device.  While this repo should be safe for S-K80's produced between 2023 and 2024, knowing the firmware issues associated with this product, I recommend using this repo (and their official drivers for that matter) only if you're willing to accept the risk of losing the device.

> NOTE (2026-07-29): Checkout tag v1.0.0 for a less useful but stable code-base.  I'm adding some CI pipelines that need to hit the main branch to switch on GH Actions and I'm too lazy to discover and apply a workaround =/

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

Every tagged release publishes a prebuilt package for `x86_64`/`aarch64` on both Linux and Windows.  Each archive contains headers, a shared and a static library, and a CMake package config.

```cmake
find_package(blink CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE blink::blink)         # shared
target_link_libraries(my_app PRIVATE blink::blink_static)  # or static
```

```cpp
#include <blink/blink.h>
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

> The public API is the `extern "C"` block in `blink.h`, but some of its signatures take `std::vector` and `std::string`.  That makes it a C++ ABI rather than a true C one, so a prebuilt binary only links against consumers built with a compatible toolchain (matching MSVC toolset and CRT on Windows, matching libstdc++ ABI on Linux).  When in doubt, build from source via the vcpkg port.

