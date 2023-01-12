add_executable(parallel_mode
    parallel_mode.c
    )

target_include_directories(parallel_mode PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# pull in common dependencies

target_link_libraries(parallel_mode 
    bme_api
    bme68x
    pico_stdlib
    hardware_i2c
)

#enable usb output
pico_enable_stdio_usb(parallel_mode 1)
pico_enable_stdio_uart(parallel_mode 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(parallel_mode)

