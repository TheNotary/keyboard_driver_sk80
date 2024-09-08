cmake_minimum_required(VERSION 3.10)
project(main)

include_directories(${CMAKE_SOURCE_DIR}/include/main)

# Find all .cpp files in the src/ directory, this isn't a best practice because it makes cmake not know what to build when things are changed supposedly?
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

set(CMAKE_CXX_STANDARD 17)

add_compile_options("/utf-8")

# Main is kind of a demo app meant for showcasing the project and testing blink.dll
# It's nice keeping main completely isolated from the source files involved with blink and vice versa
add_executable(main "src/demo/main.cpp"
	"src/demo/test_load_blink.cpp" "include/main/test_load_blink.h"
	"src/demo/push_to_light.cpp" "include/main/push_to_light.h"
	"src/demo/cycle_keyids.cpp" "include/main/cycle_keyids.h"
	"include/main/misc.h" "src/demo/choose_keyboard.cpp" "src/demo/choose_keyboard.h")

# blink.dll is what other programs will call out to in order to perform USB work
add_library(blink SHARED
	"src/blink/blink.cpp" "include/blink.h"
	"src/blink/print.cpp" "include/print.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	"src/blink/usb_functions.cpp" "include/main/usb_functions.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	"include/main/keyboards/abstract_keyboard.h"
	"include/main/keyboards/sk80/messages_sk80.h" "src/blink/keyboards/sk80/messages_sk80.cpp"
	"include/main/keyboards/rk84/constants_rk84.h"
	"src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h"
	"include/main/keyboards/rk84/key_mappings_rk84.h"
	"include/main/misc.h" "include/main/keyboards/sk80/sk80.h"
	"src/blink/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h"
	"src/blink/keyboards/keyboard_spec_factory.cpp" "include/main/keyboards/keyboard_spec_factory.h" "include/main/keyboards/rk84/messages_rk84.h")

# Add GoogleTest
add_executable(KeyboardTest
	"test/keyboard_test.cpp" "test/keyboard_rk84_test.cpp"
	"src/demo/test_load_blink.cpp" "include/main/test_load_blink.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	"src/blink/usb_functions.cpp" "include/main/usb_functions.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	"include/main/keyboards/rk84/constants_rk84.h"
	"src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h"
	"include/main/keyboards/abstract_keyboard.h"
	"include/main/misc.h" "include/main/keyboards/sk80/sk80.h"
	"src/blink/keyboards/keyboard_spec_factory.cpp" "include/main/keyboards/keyboard_spec_factory.h"
	"src/blink/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h" "include/main/keyboards/rk84/messages_rk84.h" "src/demo/choose_keyboard.cpp" "src/demo/choose_keyboard.h")

find_package(GTest CONFIG REQUIRED)
target_link_libraries(KeyboardTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
enable_testing()

add_test(NAME AllTestsInMain COMMAND KeyboardTest)

# Define the BUILD_DLL macro for the library build
target_compile_definitions(blink PRIVATE BUILD_DLL)

# fmt dependency
find_package(fmt CONFIG REQUIRED)
target_link_libraries(main PRIVATE fmt::fmt)
target_link_libraries(blink PRIVATE fmt::fmt)
target_link_libraries(KeyboardTest PRIVATE fmt::fmt)

# Google Benchmark
find_package(benchmark CONFIG REQUIRED)
add_executable(KeyboardBenchmark "benchmark/rk84_benchmark.cpp"
	"include/main/keyboards/abstract_keyboard.h" "include/main/misc.h"
	"include/main/keyboards/sk80/sk80.h"
	"src/blink/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h"
	"include/main/keyboards/rk84/messages_rk84.h")
target_link_libraries(KeyboardBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)
