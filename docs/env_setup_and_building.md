# Env Setup for Working with this C++ Repo

I planned this project to be used exclusively on a pair of windows computers.  Originally I considered using libusb which would have been cross-platform.  What I found was that libusb just didn't do very well on windows machines.  It might have worked if I replaced my keyboard driver with a special driver, but that overly complicates the installation for the app so I switched to simply calling out the the Windows API.

As that the project is my only repo demonstrating sourcing dependencies in a C++ project via vcpkg and CMake and that this is a new area of study for me, you'll find detailed steps on setting up windows-based C++ package management below.


#### Building on Terminal ("Developer Command Prompt")

At the root of the repo, run:
```
# Skip these, they were for gitbash but there are pathing issues that ruined it
#export VCPKG_ROOT="$(pwd)/vcpkg"
#export PATH=${VCPKG_ROOT}:${PATH}

set "VCPKG_ROOT=%cd%\vcpkg"
set PATH=%VCPKG_ROOT%;%PATH%

vcpkg install

# As a shortcut, run test.bat
cmake --preset release-config
cmake --build --preset release-build --target clean
cmake --build --preset release-build
ctest --preset test-release
```

Check this [guide](https://martin-fieber.de/blog/cmake-presets/#build-preset) for details.

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
  - Randomly the IDE might prompt you on the top with `CMake... delete and regenerate the cache`, click that, also try saving the CMakeLists.txt file to force a rebuild
  - Ensure your path to the vcpkg directory is correct in your CMakePresets.json file, and also that vcpkg is properly bootstrapped with `bootstrap-vcpkg.bat`
