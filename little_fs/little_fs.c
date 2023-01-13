#include <stdio.h>
#include "pico/stdlib.h"
#include "../includes/littlefs-lib/pico_hal.h"
int main(){
    stdio_init_all();
    sleep_ms(10000);
    printf("Hello World!\n");
    if (pico_mount(false) != LFS_ERR_OK) {
        printf("Error mounting FS\n");
    }

    struct pico_fsstat_t stat;
    pico_fsstat(&stat);
    printf("FS: blocks %d, block size %d, used %d\n", (int)stat.block_count, (int)stat.block_size,
           (int)stat.blocks_used);
    lfs_size_t boot_count;
    lfs_size_t rslt_fs;
    int file = pico_open("boot_count", LFS_O_CREAT | LFS_O_RDWR );
    boot_count = 0;
    rslt_fs = pico_read(file, &boot_count, sizeof(boot_count));
    if(rslt_fs < 0){
        printf("Error while reading\n");
    } 
    boot_count += 1;
    rslt_fs = pico_rewind(file);
    if(rslt_fs < 0){
        printf("Error while rewinding\n");
    }
    rslt_fs = pico_write(file, &boot_count, sizeof(boot_count));
    if(rslt_fs < 0){
        printf("Error writing\n");
    }
    pico_fflush(file);
	int pos = pico_lseek(file, 0, LFS_SEEK_CUR);
    rslt_fs = pico_close(file);
    if(rslt_fs < 0){
        printf("Error closing\n");
    }
    sleep_ms(1000);
    pico_unmount();
    printf("Boot count: %d\n", (int)boot_count);
    printf("File size (should be 4) : %d\n", pos);
}