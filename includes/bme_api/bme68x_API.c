#include "bme68x_API.h"
#include <stdio.h>


static uint8_t dev_addr;
static i2c_inst_t *i2c = i2c0;

void delay_us(uint32_t period, void *intf_ptr){
    if (period >= 1000)
        sleep_ms(period/1000);
    else
        sleep_ms(1);
}

void blink(){
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(250);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(250);
    }
}

void check_rslt_api(int8_t rslt, const char api_name[])
{
    switch (rslt)
    {
        case BME68X_OK:

            /* Do nothing */
            break;
        case BME68X_E_NULL_PTR:
            printf("API name [%s]  Error [%d] : Null pointer\r\n", api_name, rslt);
            blink();
            break;
        case BME68X_E_COM_FAIL:
            printf("API name [%s]  Error [%d] : Communication failure\r\n", api_name, rslt);
            blink();
            break;
        case BME68X_E_INVALID_LENGTH:
            printf("API name [%s]  Error [%d] : Incorrect length parameter\r\n", api_name, rslt);
            blink();
            break;
        case BME68X_E_DEV_NOT_FOUND:
            printf("API name [%s]  Error [%d] : Device not found\r\n", api_name, rslt);
            blink();
            break;
        case BME68X_E_SELF_TEST:
            printf("API name [%s]  Error [%d] : Self test error\r\n", api_name, rslt);
            blink();
            break;
        case BME68X_W_NO_NEW_DATA:
            printf("API name [%s]  Warning [%d] : No new data found\r\n", api_name, rslt);
            blink();
            break;
        default:
            printf("API name [%s]  Error [%d] : Unknown error code\r\n", api_name, rslt);
            blink();
            break;
    }
}

void check_rslt_bsec(bsec_library_return_t rslt, const char api_name[]){
    switch (rslt)
    {
        case BSEC_OK:
            break;
        case BSEC_E_DOSTEPS_INVALIDINPUT:
            printf("API name [%s]  Error [%d] : dev_id passed to bsec_do_steps() invalid range or virtual\r\n", api_name, rslt);
            blink();
            break;
        default:
            printf("API name [%s]  Error [%d] : Unknown error code\r\n", api_name, rslt);
            blink();
            break;
    }
}

BME68X_INTF_RET_TYPE bme_write(uint8_t reg, const uint8_t *buf, uint32_t nbytes, void *intf_ptr){
    int num_bytes_read = 0;
    uint8_t msg[nbytes +1];
    int8_t ret = 0;
    if(nbytes<1){
        return -1;
    }

    msg[0] = reg;
    for(int i = 0; i < nbytes; i++){
        msg[i+1] = buf[i];
    }

    ret = i2c_write_blocking(i2c, dev_addr, msg, (nbytes+1), false);
    if(ret > 0)
        return 0;
    else
        return -1;
}


BME68X_INTF_RET_TYPE bme_read(uint8_t reg, uint8_t *buf, uint32_t nbytes, void *intf_ptr){
    int8_t num_bytes_read = 0;
    int8_t ret = 0;

    if(nbytes < 0){
        return num_bytes_read;
    }
    i2c_write_blocking(i2c, dev_addr, &reg, 1, true);
    num_bytes_read = i2c_read_blocking(i2c, dev_addr, buf, nbytes, false);

    if(num_bytes_read > 0)
        return 0;
    else 
        return -1;
}

int8_t bme_interface_init(struct bme68x_dev *bme, uint8_t intf){
    //i2c interface configuration
    if (intf == BME68X_I2C_INTF){
        dev_addr = BME68X_I2C_ADDR_LOW;
        bme->read = bme_read;
        bme->write = bme_write;
        bme->intf = BME68X_I2C_INTF;
        
        i2c_init(i2c, 400*1000);
        gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
        gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    }
    /* Bus configuration : SPI */
    else if (intf == BME68X_SPI_INTF){
        //TODO: spi interface not used yet so not implemented
    }
    bme->intf_ptr = &dev_addr;
    bme->amb_temp = 20;
    bme->delay_us = delay_us;
}

uint8_t get_dev_addr(){
    return dev_addr;
}

i2c_inst_t* get_i2c(){
    return i2c;
}