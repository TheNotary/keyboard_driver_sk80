# XVX S-K80 Keyboard Driver for LED Control

[![windows](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/windows.yml/badge.svg)](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/windows.yml)

> Warning: One keeb was lost during the development of this driver.  XVX S-K80 and company (Womier seems to be an alternate name) does not ship with a very robust firmware and has open vulnerabilities that allow any non-root program on your computer to permenently destroy the device.  If your computer emmits the wrong packets to the keyboard, it could actually brick the device.  While this repo should be safe for S-K80's produced between 2023 and 2024, knowing the firmware issues associated with this product, I recommend using this repo (and their official drivers for that matter) only if you're willing to accept the risk of losing the device.

This project builds `blink.dll` a userspace driver that can control the LEDs on an XVX S-K80 mechanical keyboard.  It happens to also support the RK84 by Royal Kludge.
To communicate with the keyboard, this project leverages the Windows API, meaning it shouldn't require any installation to be run on a windows machine, but also won't work for linux.  Please leave either a note or a PR if you'd like linux supported.


#### Building on Terminal ("Developer Command Prompt")

To conduct the build and execute the tests, at the root of the repo, run:

```
setup.bat
```

For more notes about vcpkg, CMake, and doing the build on a windows-based C++ environment see [here](docs/env_setup_and_building.md).
