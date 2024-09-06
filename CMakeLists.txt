cmake_minimum_required(VERSION 3.10)
project(main)

include_directories(${CMAKE_SOURCE_DIR}/include/main)

# Find all .cpp files in the src/ directory, this isn't a best practice because it makes cmake not know what to build when things are changed supposedly?
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp") 

set(CMAKE_CXX_STANDARD 17)

add_compile_options("/utf-8")

# Main is kind of a demo app meant for showcasing the project and testing blink.dll
# It's nice keeping main completely isolated from the source files involved with blink and vice versa
add_executable(main "src/main.cpp"
	"src/test_load_blink.cpp" "include/main/test_load_blink.h"
	"src/push_to_light.cpp" "include/main/push_to_light.h"
	"src/cycle_keyids.cpp" "include/main/cycle_keyids.h" 
	"include/main/misc.h")

# blink.dll is what other programs will call out to in order to perform USB work
add_library(blink SHARED
	"src/blink/blink.cpp" "include/blink.h"
	"src/blink/print.cpp" "include/print.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	"src/blink/usb_functions.cpp" "include/main/usb_functions.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	"include/main/keyboards/keyboard_base.h" 
	"include/main/keyboards/sk80/messages_sk80.h" "src/blink/keyboards/sk80/messages_sk80.cpp"
	"src/blink/keyboards/rk84/messages_rk84.cpp" "include/main/keyboards/rk84/messages_rk84.h" 
	"src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h" 
	"include/main/keyboards/keyboard_base.h" 
	"include/main/misc.h")

# Add GoogleTest
add_executable(KeyboardTest
	"test/keyboard_test.cpp" "test/keyboard_rk84_test.cpp"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	"src/blink/usb_functions.cpp" "include/main/usb_functions.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	"src/blink/keyboards/rk84/messages_rk84.cpp" "include/main/keyboards/rk84/messages_rk84.h" 
	"src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h" 
	 "include/main/keyboards/keyboard_base.h" 
	"include/main/misc.h")

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
add_executable(KeyboardBenchmark "benchmark/rk84_benchmark.cpp"  "include/main/keyboards/keyboard_base.h" "include/main/misc.h")
target_link_libraries(KeyboardBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)
