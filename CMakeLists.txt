cmake_minimum_required(VERSION 3.10)
project(main)

include_directories(${CMAKE_SOURCE_DIR}/include)
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp") # Find all .cpp files in the src/ directory

add_executable(main ${SOURCES})

# fmt dependency
find_package(fmt CONFIG REQUIRED)
target_link_libraries(main PRIVATE fmt::fmt)

# libusb dependency
#find_package(pkgconfig REQUIRED)
#pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)
#target_link_libraries(main PRIVATE PkgConfig::libusb)
