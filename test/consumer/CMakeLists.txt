####################################################################################
# Downstream consumer smoke test.
#
# Not part of the main build. Every leg of the pipeline configures this against
# the staged install tree, which is the only real proof that the exported
# package (headers + libraries + blinkConfig.cmake) is actually usable.
####################################################################################
cmake_minimum_required(VERSION 3.16)
project(blink_consumer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(blink CONFIG REQUIRED)

message(STATUS "Found blink ${blink_VERSION}")

enable_testing()

# A package may ship one linkage or both, depending on how it was built.
if(TARGET blink::blink)
    add_executable(consumer_shared main.cpp)
    target_link_libraries(consumer_shared PRIVATE blink::blink)
    add_test(NAME consumer_shared COMMAND consumer_shared)

    if(WIN32)
        # The import library is enough to link, but the .dll has to sit next to
        # the executable for the test run to start.
        add_custom_command(TARGET consumer_shared POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "$<TARGET_FILE:blink::blink>" "$<TARGET_FILE_DIR:consumer_shared>")
    endif()
endif()

if(TARGET blink::blink_static)
    add_executable(consumer_static main.cpp)
    target_link_libraries(consumer_static PRIVATE blink::blink_static)
    add_test(NAME consumer_static COMMAND consumer_static)
endif()
