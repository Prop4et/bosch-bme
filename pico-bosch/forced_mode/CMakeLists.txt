cmake_minimum_required(VERSION 3.12)
# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(forced_mode C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(forced_mode
    main.c
    )

target_include_directories(forced_mode PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# pull in common dependencies

target_link_libraries(forced_mode 
    pico_stdlib
    hardware_i2c
    bme68x
    bme_api
)

#enable usb output
pico_enable_stdio_usb(forced_mode 1)
pico_enable_stdio_uart(forced_mode 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(forced_mode)

