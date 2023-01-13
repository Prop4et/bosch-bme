add_library(littlefs-lib 
    lfs.c
    lfs.h
    lfs_util.h
    pico_hal.c
    pico_hal.h
)

target_link_libraries(littlefs-lib
    hardware_flash
    hardware_sync
    pico_time
    hardware_irq
)