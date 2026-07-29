####################################################################################
# Downstream consumer smoke test.
#
# Not part of the main build. Every leg of the pipeline configures this against
# the staged install tree, which is the only real proof that the exported
# package (headers + libraries + blinkConfig.cmake) is actually usable.
####################################################################################
cmake_minimum_required(VERSION 3.16)
project(blink_consumer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(blink CONFIG REQUIRED)

message(STATUS "Found blink ${blink_VERSION}")

enable_testing()

# A package may ship one linkage or both, depending on how it was built.
if(TARGET blink::blink)
    add_executable(consumer_shared main.cpp)
    target_link_libraries(consumer_shared PRIVATE blink::blink)
    add_test(NAME consumer_shared COMMAND consumer_shared)

    # The real portability check. blink.h claims to be a C ABI, so a C compiler
    # has to accept it; only a C compiler can catch C++-isms that would make the
    # header unusable from Rust and other FFI consumers.
    add_executable(consumer_c main.c)
    target_link_libraries(consumer_c PRIVATE blink::blink)
    add_test(NAME consumer_c COMMAND consumer_c)

    if(WIN32)
        # The import library is enough to link, but the .dll has to sit next to
        # the executable for the test run to start.
        foreach(consumer_target consumer_shared consumer_c)
            add_custom_command(TARGET ${consumer_target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                        "$<TARGET_FILE:blink::blink>" "$<TARGET_FILE_DIR:${consumer_target}>")
        endforeach()
    endif()
endif()

if(TARGET blink::blink_static)
    add_executable(consumer_static main.cpp)
    target_link_libraries(consumer_static PRIVATE blink::blink_static)
    add_test(NAME consumer_static COMMAND consumer_static)

    add_executable(consumer_static_c main.c)
    target_link_libraries(consumer_static_c PRIVATE blink::blink_static)
    # The archive is C++; the C driver would not pull in the C++ runtime.
    set_target_properties(consumer_static_c PROPERTIES LINKER_LANGUAGE CXX)
    add_test(NAME consumer_static_c COMMAND consumer_static_c)
endif()
