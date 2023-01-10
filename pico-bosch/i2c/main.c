/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "bme68x/bme68x_defs.h"

int reg_write(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes);

int reg_read(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes);

int main() {
    sleep_ms(10000);
    printf("Starting I2C comms\n");
    const uint sda_pin = 8;
    const uint scl_pin = 9;

    i2c_inst_t *i2c = i2c0;

    uint8_t data[6];

    stdio_init_all();
    
    i2c_init(i2c, 400*1000);

    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    while(true){
        reg_read(i2c, BME68X_I2C_ADDR_LOW, BME68X_REG_CHIP_ID, data, 1);
        if(data[0] != BME68X_CHIP_ID){
            printf("Cannot communicate with BME688\n");
            printf("CHIP_ID: %x\t ID_READ: %x\n", BME68X_CHIP_ID, data[0]);
        }
        else{
            printf("ID_READ: %x", data[0]);
        }
        sleep_ms(5000);
    }
}

int reg_write(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes){
    int num_bytes_read = 0;
    uint8_t msg[nbytes +1];

    if(nbytes<1){
        return 0;
    }

    msg[0] = reg;
    for(int i = 0; i < nbytes; i++){
        msg[i+1] = buf[i];
    }

    i2c_write_blocking(i2c, addr, msg, (nbytes+1), false);
}

int reg_read(i2c_inst_t *i2c, const uint addr, const uint8_t reg, uint8_t *buf, const uint8_t nbytes){
    int num_bytes_read = 0;

    if(nbytes < 0){
        return 0;
    }

    i2c_write_blocking(i2c, addr, &reg, 1, true);
    num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);
    return num_bytes_read;
}