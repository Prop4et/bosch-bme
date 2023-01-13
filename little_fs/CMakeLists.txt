add_executable(little_fs
    little_fs.c
    )

target_include_directories(little_fs PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# pull in common dependencies
target_link_libraries(little_fs 
    pico_stdlib
)


#enable usb output
pico_enable_stdio_usb(little_fs 1)
pico_enable_stdio_uart(little_fs 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(little_fs)

