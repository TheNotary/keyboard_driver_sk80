# XVX S-K80 Keyboard Driver for LED Control

This project builds an exe that can control the LEDs on an XVX S-K80 mechanical keyboard.  As that the project also demonstrates sourcing dependencies in a C++ project via vcpkg and CMake and that this is a new area of study for me, you'll find detailed steps on setting up windows-based C++ package management below.

This project uses the Windows API for interacting with USB, meaning it shouldn't require any installation to be run on a windows machine, but also won't work for linux.  Please leave a note if you'd like linux supported.


#### Dependency Overview

###### Environment Setup
- Visual Studio 2022: An IDE that installs a ton of C++ build tools
- Cygwin (I think I needed some package from this to get past an error?)
- CMake: A cross platform C++ build tool for scripting the compilation pipeline
- vcpkg: msft's package manager for C++
  - See this [tutorial](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-cmd)
  - The tut seemed broken and but may just require MinGW or simply running in the "Developer Command Prompt"
- MinGW: A package manager allowing you to install gcc, etc. on windows to `C:\MinGW\bin`
- pkg-config: Another package manager required by libusb
  - Follow these great [instructions](https://stackoverflow.com/a/22363820)

###### C++ Module Dependencies

- fmt: A dependency used in main.cpp to print to the screen.
  - List this package in `vcpkg.json`
  - Install with `vcpkg install`
  - Read outputs of install and copy/ paste codes into `CMakeLists.txt` to setup linking
- libusb: A more complicated dependency we use in main.cpp for interfacing with USB
  - List this package in `vcpkg.json`
  - Install with `vcpkg install`
  - You will note the outputs indicate using `find_package(pkgconfig REQUIRED)`
  - Copy/ paste codes into CMakeLists.txt
  - Make sure pkg-config is installed
- If anything ever goes wrong with the build system, and re-run:
  - delete the `build/` folder manually
  - `cmake --preset=default`
  - `cmake --build build`
- Getting the error "The following configuration files were considered but not accepted:"
  - Don't forget to use the "Developer Command Prompt"
  - Randomly the IDE might prompt you on the top with `CMake... delete and regenerate the cache`, click that
