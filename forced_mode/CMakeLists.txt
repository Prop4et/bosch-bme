add_executable(forced_mode
    forced_mode.c
    )

target_include_directories(forced_mode PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# pull in common dependencies
SET_TARGET_PROPERTIES(bsec PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(forced_mode 
    bme68x
    bme_api
    bsec
    pico_stdlib
    hardware_i2c
    algobsec
)


#enable usb output
pico_enable_stdio_usb(forced_mode 1)
pico_enable_stdio_uart(forced_mode 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(forced_mode)

