cmake_minimum_required(VERSION 3.10)
project(main)

include_directories(${CMAKE_SOURCE_DIR}/include/main)
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp") # Find all .cpp files in the src/ directory

# Define the build outputs, main.exe and blink.dll
#add_executable(main ${SOURCES} "src/blink/print.cpp" "include/print.h")

add_library(blink SHARED 
	"src/blink/blink.cpp" "include/blink.h" 
	"src/blink/print.cpp" "include/print.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	"src/blink/usb_functions.cpp" "include/main/usb_functions.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
 "include/main/messages_sk80.h" "src/blink/messages_sk80.cpp")

add_executable(main "src/main.cpp" 
	"src/test_load_blink.cpp" "include/main/test_load_blink.h"
 "include/main/messages_sk80.h" "src/blink/messages_sk80.cpp")

# Define the BUILD_DLL macro for the library build
target_compile_definitions(blink PRIVATE BUILD_DLL)

# fmt dependency
find_package(fmt CONFIG REQUIRED)
target_link_libraries(main PRIVATE fmt::fmt)
target_link_libraries(blink PRIVATE fmt::fmt)

# libusb dependency
#find_package(pkgconfig REQUIRED)
#pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
#target_link_libraries(main PRIVATE PkgConfig::libusb)
