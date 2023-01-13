cmake_minimum_required(VERSION 3.12)
# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(pico_bosch_cmake C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_subdirectory(includes)
add_subdirectory(forced_mode)
add_subdirectory(parallel_mode)
add_subdirectory(i2c)
add_subdirectory(bsec_use)
add_subdirectory(little_fs)