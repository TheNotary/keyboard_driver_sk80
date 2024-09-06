# XVX S-K80 Keyboard Driver for LED Control

This project builds an exe that can control the LEDs on an XVX S-K80 mechanical keyboard.  As that the project also demonstrates sourcing dependencies in a C++ project via vcpkg and CMake and that this is a new area of study for me, you'll find detailed steps on setting up windows-based C++ package management below.

This project uses the Windows API for interacting with USB, meaning it shouldn't require any installation to be run on a windows machine, but also won't work for linux.  Please leave a note if you'd like linux supported.


#### Building on Terminal ("Developer Command Prompt")

To conduct the build and execute the tests, at the root of the repo, run:
```
test.bat
```

More note available [here](docs/env_setup_and_building.md).
