# XVX S-K80 Keyboard Driver for LED Control

> Disclaimer: XVX S-K80 and company (womier seems to be an alternate name) does not ship with very good firmware and has open vulnerabilities that allow any non-root program on your computer permenently destroy the device.  If your computer emmits the wrong packets to the keyboard, it could actually brick the device.  This repo is safe for S-k80's produced between 2023 and 2024 but knowing the issues with firmware released by this company, I don't recommend testing this repo (or their official drivers for that matter) unless you're willing to risk the device.  

This project builds `blink.dll` a userspace driver that can control the LEDs on an XVX S-K80 mechanical keyboard.  It happens to also support the RK84 by Royal Kludge.  
To communicate with the keyboard, this project leverages the Windows API, meaning it shouldn't require any installation to be run on a windows machine, but also won't work for linux.  Please leave a note if you'd like linux supported.


#### Building on Terminal ("Developer Command Prompt")

To conduct the build and execute the tests, at the root of the repo, run:
```
setup.bat
```

For more notes about vcpkg, CMake, and doing the build on a windows-based C++ environment see [here](docs/env_setup_and_building.md).
