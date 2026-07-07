# Env Setup for Working with this C++ Repo

This project supports both Windows and Linux.  On Windows it uses the native HID APIs (SetupAPI + hidsdi); on Linux it uses [hidapi](https://github.com/libusb/hidapi) (hidraw backend) for equivalent functionality.  Dependencies are managed via vcpkg and the build is driven by CMake with Ninja.


## Building on Linux

Prerequisites: a C++ compiler (g++ or clang++), CMake, Ninja, and pkg-config.

```bash
./setup.sh
```

This script bootstraps vcpkg, installs dependencies (including hidapi), configures, builds, and runs tests.  Build artifacts land in `build/linux-debug/`.

To build manually:

```bash
export VCPKG_ROOT="$(pwd)/vcpkg"
export PATH="$VCPKG_ROOT:$PATH"
vcpkg install

cmake --preset linux-debug
cmake --build --preset linux-debug-build
ctest --preset test-linux-debug
```

#### udev Rules

The keyboard's hidraw device is only accessible to root by default.  Install the included udev rule to allow non-root access:

```bash
sudo cp 99-keyboard-leds.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

#### Integration Tests

Set the env variable `KEYBOARD_ATTACHED=1` to enable tests that require a physical keyboard connected.


## Building on Windows ("Developer Command Prompt")

At the root of the repo, run:

```
setup.bat
```

Or manually:

```
set "VCPKG_ROOT=%cd%\vcpkg"
set PATH=%VCPKG_ROOT%;%PATH%

vcpkg install

cmake --preset release-config
cmake --build --preset release-build
ctest --preset test-release
```

Check this [guide](https://martin-fieber.de/blog/cmake-presets/#build-preset) for details on CMake presets.

#### Dependency Overview (Windows)

###### Environment Setup
- Visual Studio 2022: An IDE that installs a ton of C++ build tools
- CMake: A cross platform C++ build tool for scripting the compilation pipeline
- vcpkg: msft's package manager for C++
  - See this [tutorial](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-cmd)

###### Troubleshooting
- If anything goes wrong with the build system:
  - Delete the `build/` folder manually
  - Re-run `cmake --preset=default` then `cmake --build build`
- Getting the error "The following configuration files were considered but not accepted:"
  - Don't forget to use the "Developer Command Prompt"
  - Ensure your path to the vcpkg directory is correct in your CMakePresets.json file, and also that vcpkg is properly bootstrapped with `bootstrap-vcpkg.bat`
