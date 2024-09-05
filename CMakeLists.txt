cmake_minimum_required(VERSION 3.10)
project(main)

include_directories(${CMAKE_SOURCE_DIR}/include/main)
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp") # Find all .cpp files in the src/ directory

set(CMAKE_CXX_STANDARD 17)

add_compile_options("/utf-8")

# Define the build outputs, main.exe and blink.dll
#add_executable(main ${SOURCES} "src/blink/print.cpp" "include/print.h")

add_library(blink SHARED
	"src/blink/blink.cpp" "include/blink.h"
	"src/blink/print.cpp" "include/print.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	"src/blink/usb_functions.cpp" "include/main/usb_functions.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	 "include/main/messages_sk80.h" "src/blink/messages_sk80.cpp"
	 "src/blink/keyboards/rk84/messages_rk84.cpp" "include/main/keyboards/rk84/messages_rk84.h" "src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h")

add_executable(main "src/main.cpp"
	"src/test_load_blink.cpp" "include/main/test_load_blink.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	"include/main/messages_sk80.h" "src/blink/messages_sk80.cpp"
	"src/blink/keyboards/rk84/messages_rk84.cpp" "include/main/keyboards/rk84/messages_rk84.h"
	"src/push_to_light.cpp" "include/main/push_to_light.h"
	"src/cycle_keyids.cpp" "include/main/cycle_keyids.h" "src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h")

#add_executable(KeyboardTest ${SOURCES} test/keyboard_test.cpp src/blink/keyboard.cpp)

# Add GoogleTest
find_package(GTest CONFIG REQUIRED)
add_executable(KeyboardTest
	"test/keyboard_test.cpp" "test/keyboard_rk84_test.cpp"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	"src/blink/usb_functions.cpp" "include/main/usb_functions.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	"src/blink/keyboards/rk84/messages_rk84.cpp" "include/main/keyboards/rk84/messages_rk84.h" "src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h")
target_link_libraries(KeyboardTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
enable_testing()

#add_test(KeyboardTest KeyboardTest)
add_test(NAME AllTestsInMain COMMAND KeyboardTest)
#add_test(AllTestsInMain main)

# Define the BUILD_DLL macro for the library build
target_compile_definitions(blink PRIVATE BUILD_DLL)

# fmt dependency
find_package(fmt CONFIG REQUIRED)
target_link_libraries(main PRIVATE fmt::fmt)
target_link_libraries(blink PRIVATE fmt::fmt)
target_link_libraries(KeyboardTest PRIVATE fmt::fmt)

# Google Benchmark
find_package(benchmark CONFIG REQUIRED)
add_executable(KeyboardBenchmark "benchmark/rk84_benchmark.cpp")
target_link_libraries(KeyboardBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)

# libusb dependency
#find_package(pkgconfig REQUIRED)
#pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
#target_link_libraries(main PRIVATE PkgConfig::libusb)
