# XVX S-K80 Keyboard Driver for LED Control

[![windows](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/windows.yml/badge.svg)](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/windows.yml)
[![linux](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/linux.yml/badge.svg)](https://github.com/TheNotary/keyboard_driver_sk80/actions/workflows/linux.yml)

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

