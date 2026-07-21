cmake_minimum_required(VERSION 3.10)
project(main)

include_directories("${CMAKE_SOURCE_DIR}/include/main")

# Find all .cpp files in the src/ directory, this isn't a best practice because it makes cmake not know what to build when things are changed supposedly?
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_compile_options("/utf-8")
endif()

if(WIN32)
    set(USB_SOURCES "src/blink/usb_functions_win.cpp")
else()
    set(USB_SOURCES "src/blink/usb_functions_linux.cpp")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(HIDAPI REQUIRED hidapi-libusb)
endif()

# Main is kind of a demo app meant for showcasing the project and testing blink.dll
# It's nice keeping main completely isolated from the source files involved with blink and vice versa
add_executable(main "src/demo/main.cpp"
	"src/demo/blink_loader.cpp" "src/demo/blink_loader.h"
	"src/demo/activities/push_to_light.cpp" "src/demo/activities/push_to_light.h"
	"src/demo/activities/cycle_keyids.cpp" "src/demo/activities/cycle_keyids.h"
	"include/main/misc.h" "src/demo/choose_keyboard.cpp" "src/demo/choose_keyboard.h"
	"src/demo/choose_activity.cpp" "src/demo/choose_activity.h"
	"src/demo/util.cpp" "src/demo/util.h"
	"src/demo/activities/test_single_key.cpp" "src/demo/activities/test_single_key.h")
target_include_directories(main PRIVATE "${CMAKE_SOURCE_DIR}/src/demo")


# blink.dll is what other programs will call out to in order to perform USB work
add_library(blink SHARED
	"src/blink/blink.cpp" "include/main/blink.h"
	"src/blink/print.cpp" "include/main/print.h"
	"src/blink/messages.cpp" "include/main/messages.h"
	${USB_SOURCES} "include/main/usb_functions.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	"include/main/keyboards/abstract_keyboard.h"
    "include/main/keyboards/sk80/sk80.h"

	"include/main/keyboards/sk80/constants_sk80.h" "include/main/keyboards/sk80/messages_sk80.h" "include/main/keyboards/sk80/key_mappings_sk80.h"
    "include/main/keyboards/rk84/constants_rk84.h"
	"src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h"
	"include/main/keyboards/rk84/key_mappings_rk84.h"
	"include/main/misc.h"
	"src/blink/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h"
	"src/blink/keyboards/keyboard_spec_factory.cpp" "include/main/keyboards/keyboard_spec_factory.h" "include/main/keyboards/rk84/messages_rk84.h" "src/blink/keyboards/sk80/sk80.cpp"
	"src/blink/keyboard_lcd.cpp" "include/main/keyboard_lcd.h"
	"src/blink/usb_io_real.cpp" "include/main/usb_io.h")

# gif_to_sk80 uploads GIF/image files to the SK80 LCD display
add_executable(gif_to_sk80 "src/gif_to_sk80/main.cpp")
target_link_libraries(gif_to_sk80 PRIVATE blink)

# Add GoogleTest
add_executable(KeyboardTest
	"test/keyboard_test.cpp" "test/keyboard_rk84_test.cpp" "test/keyboard_sk80_test.cpp"
	"src/demo/blink_loader.cpp" "src/demo/blink_loader.h"
	"src/blink/keyboard.cpp" "include/main/keyboard.h"
	${USB_SOURCES} "include/main/usb_functions.h"
	"src/blink/messages.cpp" "include/main/messages.h"
    "include/main/keyboards/sk80/sk80.h"
	"include/main/keyboards/sk80/constants_sk80.h" "include/main/keyboards/sk80/messages_sk80.h" "include/main/keyboards/sk80/key_mappings_sk80.h"
    "src/blink/keyboards/rk84/rk84.cpp" "include/main/keyboards/rk84/rk84.h"
    "include/main/keyboards/rk84/constants_rk84.h" "include/main/keyboards/rk84/messages_rk84.h"
	"include/main/keyboards/abstract_keyboard.h"
	"include/main/misc.h"
	"src/blink/keyboards/keyboard_spec_factory.cpp" "include/main/keyboards/keyboard_spec_factory.h"
	"src/blink/keyboards/known_keyboards.cpp" "include/main/keyboards/known_keyboards.h"
    "src/demo/choose_keyboard.cpp" "src/demo/choose_keyboard.h" "src/demo/choose_activity.cpp" "src/demo/choose_activity.h"
    "src/demo/util.cpp" "src/demo/util.h" "src/demo/activities/test_single_key.cpp" "src/demo/activities/test_single_key.h" "src/blink/keyboards/sk80/sk80.cpp"
	"test/gif_to_sk80_test.cpp"
	"src/blink/keyboard_lcd.cpp" "include/main/keyboard_lcd.h" "include/main/usb_io.h"
	"src/blink/usb_io_real.cpp")
target_include_directories(KeyboardTest PRIVATE "${CMAKE_SOURCE_DIR}/test")

find_package(GTest CONFIG REQUIRED)
target_link_libraries(KeyboardTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
target_compile_definitions(KeyboardTest PRIVATE TEST_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# stb for image loading/resizing (used by keyboard_lcd)
find_path(STB_INCLUDE_DIRS "stb_image.h")
if(STB_INCLUDE_DIRS)
    target_include_directories(blink PRIVATE ${STB_INCLUDE_DIRS})
    target_include_directories(KeyboardTest PRIVATE ${STB_INCLUDE_DIRS})
else()
    find_package(Stb REQUIRED)
    target_include_directories(blink PRIVATE ${Stb_INCLUDE_DIR})
    target_include_directories(KeyboardTest PRIVATE ${Stb_INCLUDE_DIR})
endif()

if(NOT WIN32)
    target_include_directories(blink PRIVATE ${HIDAPI_INCLUDE_DIRS})
    target_link_libraries(blink PRIVATE ${HIDAPI_LIBRARIES})
    target_include_directories(KeyboardTest PRIVATE ${HIDAPI_INCLUDE_DIRS})
    target_link_libraries(KeyboardTest PRIVATE ${HIDAPI_LIBRARIES})
    target_link_libraries(main PRIVATE dl)
    target_link_libraries(KeyboardTest PRIVATE dl)
    target_link_libraries(gif_to_sk80 PRIVATE dl)
endif()

enable_testing()

add_test(NAME AllTestsInMain COMMAND KeyboardTest)

# Define the BUILD_DLL macro for the library build
target_compile_definitions(blink PRIVATE BUILD_DLL)

# Google Benchmark
find_package(benchmark CONFIG REQUIRED)
add_executable(KeyboardBenchmark "benchmark/rk84_benchmark.cpp")
target_link_libraries(KeyboardBenchmark PRIVATE benchmark::benchmark benchmark::benchmark_main)

