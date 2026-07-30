# Env Setup for Working with this C++ Repo

This project supports both Windows and Linux.  On Windows it uses the native HID APIs (SetupAPI + hidsdi); on Linux it uses [hidapi](https://github.com/libusb/hidapi) for equivalent functionality.  Dependencies are managed via vcpkg and the build is driven by CMake with Ninja.

hidapi is declared in `vcpkg.json` and is built by vcpkg on Linux, so no distro package is required.  If a CMake package config for hidapi is not found, the build falls back to `pkg-config` against a system `libhidapi-dev`, which is what distro packagers will normally want.


## Build targets and options

| Option | Default | Purpose |
| --- | --- | --- |
| `BLINK_BUILD_SHARED` | `ON` | Build `libblink.so` / `blink.dll` |
| `BLINK_BUILD_STATIC` | `ON` | Build `libblink.a` / `blink_static.lib` |
| `BLINK_BUILD_TESTS` | `ON` | Build `KeyboardTest` (requires the `tests` vcpkg feature) |
| `BLINK_BUILD_BENCHMARKS` | `ON` | Build `KeyboardBenchmark` (requires the `benchmarks` vcpkg feature) |
| `BLINK_BUILD_DEMO` | `ON` | Build the interactive `main` demo application |

`BLINK_VERSION` overrides the version stamped onto the library; the release pipeline passes the git tag.


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


## The multi-arch pipeline

Three workflows under `.github/workflows/`:

| Workflow | Trigger | Role |
| --- | --- | --- |
| `build.yaml` | `workflow_call` only | Builds, tests and packages exactly one target |
| `ci.yaml` | push to `main`, pull requests, `workflow_call` | Fans `build.yaml` out over the target matrix |
| `release.yaml` | `v*.*.*` tags, manual dispatch | Resolves the version, calls `ci.yaml`, publishes the release |

Targets:

| Target | Runner | vcpkg triplet |
| --- | --- | --- |
| `x86_64-linux-gnu` | `ubuntu-22.04` | `x64-linux` |
| `aarch64-linux-gnu` | `ubuntu-24.04-arm` | `arm64-linux` |
| `x86_64-windows-msvc` | `windows-latest` | `x64-windows` |
| `aarch64-windows-msvc` | `windows-11-arm` | `arm64-windows` |

The arm64 runners are free for public repositories only; a private repo needs larger runners for those two legs.

Every leg uses the `ci` CMake preset.  The preset points `CMAKE_TOOLCHAIN_FILE` at the vcpkg checkout bundled in this repo, so it needs no environment set up at all; CI passes `-DVCPKG_TARGET_TRIPLET` on the command line to cover all four targets, and left unset vcpkg detects the host triplet.  Do not move that into the preset as `$env{VCPKG_TARGET_TRIPLET}` — when the variable is unset it expands to an empty cache entry and vcpkg rejects it as an invalid triplet.

After building and running `ctest`, each leg installs to a staging prefix and then configures `test/consumer` against it — a standalone project that does nothing but `find_package(blink CONFIG REQUIRED)` and link both library targets, from both C++ and C99.  That step is what actually proves the published package is usable, so keep it green.  The x86_64 legs additionally build and test the Rust crates in `rust/` against the same staging prefix, which is what keeps `blink-sys`' checked-in FFI declarations honest.

To reproduce a CI leg locally:

```bash
# No VCPKG_ROOT needed: the preset uses the vcpkg checkout in this repo.
# Drop the -D to build for the host triplet.
cmake --preset ci -DVCPKG_TARGET_TRIPLET=x64-linux
cmake --build --preset ci
ctest --preset ci

cmake --install build/ci --prefix "$(pwd)/stage"
cmake -S test/consumer -B build/consumer -G Ninja \
  -DCMAKE_PREFIX_PATH="$(pwd)/stage;$(pwd)/build/ci/vcpkg_installed/x64-linux"
cmake --build build/consumer
ctest --test-dir build/consumer --output-on-failure

# The Rust half, linked against that same staging prefix.
cd rust
BLINK_LIB_DIR="$(pwd)/../stage/lib" BLINK_INCLUDE_DIR="$(pwd)/../stage/include" \
  cargo test --workspace --no-default-features --features static
```

#### Cutting a release

Push a `vX.Y.Z` tag.  The tag is the source of truth for the version, so `CMakeLists.txt` never needs a manual bump beforehand.  The release job publishes a per-target archive of the install tree, a `SHA256SUMS`, and `blink-vcpkg-port.tar.gz` — the overlay port in `ports/blink/` rendered against the tag with a real SHA512.  Use `workflow_dispatch` to exercise the build and packaging path without publishing anything.

#### Exported symbols

The shared library is built with `-fvisibility=hidden` and only the `extern "C"` API in `blink.h` is exported.  vcpkg hands the build static hidapi/libusb archives, so `--exclude-libs,ALL` is also applied to stop `libblink.so` from re-exporting the whole libusb ABI.  To check:

```bash
nm -D --defined-only stage/lib/libblink.so | grep ' T '
```

Every name that prints must start with `blink_`; CI fails the build otherwise.  That guarantee is what makes the shared library callable from any language with a C FFI, which is what the Rust crates in `rust/` rely on.

`gif_to_sk80` drives the library's internal C++ classes rather than that API, so it links the static archive.
