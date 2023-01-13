add_executable(bsec_use
    bsec_use.c
    bsec_use.h
    )

target_include_directories(bsec_use PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# pull in common dependencies
SET_TARGET_PROPERTIES(bsec PROPERTIES LINKER_LANGUAGE C)

add_library(algobsec STATIC IMPORTED)
set_property(TARGET algobsec PROPERTY IMPORTED_LOCATION /home/francesco/pico/pico-bosch/includes/bsec/libalgobsec.a)


target_link_libraries(bsec_use 
    bme68x
    bme_api
    bsec
    pico_stdlib
    hardware_i2c
    algobsec
    littlefs-lib
)


#enable usb output
pico_enable_stdio_usb(bsec_use 1)
pico_enable_stdio_uart(bsec_use 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(bsec_use)

