#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../includes/bme68x/bme68x_defs.h"
#include "../includes/bme68x/bme68x.h"
#include "../includes/bme_api/bme68x_API.h"

int main() {

    struct bme68x_dev bme;
    struct bme68x_data data;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    uint16_t sample_count = 1;
    uint32_t time_ms = 0;
    int8_t rslt;

    stdio_init_all();
    sleep_ms(10000);
    printf("Init BME688\n");
    bme_interface_init(&bme, BME68X_I2C_INTF);
    uint8_t data_id[8];

    //read device id after init to see if everything works for now and to check that the device communicates with I2C
    bme.read(BME68X_REG_CHIP_ID, data_id, 1, bme.intf_ptr);
    if(data_id[0] != BME68X_CHIP_ID){
        printf("Cannot communicate with BME688\n");
        printf("CHIP_ID: %x\t ID_READ: %x\n", BME68X_CHIP_ID, data_id[0]);
    }
    else{
        bme.chip_id = data_id[0];
        printf("Connection valid, DEVICE_ID: %x\n", bme.chip_id);
    }

    sleep_ms(500);
    
    //initialize the structure with all the parameters by reading from the registers
    rslt = bme68x_init(&bme);
    check_err(rslt, "INIT");

    /*
        Set oversampling for measurements
        1x oversampling for humidity
        16x oversampling for pressure
        2x oversampling for temperature
    */
    conf.os_hum = BME68X_OS_1X;
    conf.os_pres = BME68X_OS_16X;
    conf.os_temp = BME68X_OS_2X;
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;

    /*  
        Set the remaining gas sensor settings and link the heating profile 
        enable the heater plate
        set the temperature plate to 300°C
        set the duration to 100 ms
    */
    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp = 300;
    heatr_conf.heatr_dur = 100;

    
    rslt = bme68x_set_conf(&conf, &bme);
    check_err(rslt, "bme68x_set_conf");

    rslt = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);
    check_err(rslt, "bme68x_set_heatr_conf");

    uint8_t n_fields;
    uint32_t del_period;

    while(1){
        rslt = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme); 
        check_err(rslt, "bme68x_set_op_mode");

        del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme) + (heatr_conf.heatr_dur * 1000);

        bme.delay_us(del_period, bme.intf_ptr);

        rslt = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);
        check_err(rslt, "bme68x_get_data");
        
        if (rslt == BME68X_OK){
            if (! ((data.idac != 0x00) && (data.idac != 0xFF) ||
                (data.status & BME68X_GASM_VALID_MSK))){
                check_err(BME68X_E_SELF_TEST, "bme68x_get_data");
            }
        }
        printf("T: %.2f degC\n", data.temperature);
        sleep_ms(10);//sleeps for flushing the stdout
        printf("P: %.2f Pa\n", data.pressure);
        sleep_ms(10);
        printf("H %.2f rH\n", data.humidity);
        sleep_ms(10);
        printf("Gas: %f ohms\n", data.gas_resistance);
        sleep_ms(10);
        printf("Status: 0x%x\n", data.status);
        //sensor goes automatically to sleep
        sleep_ms(30000);
    }
    //printf("Self test result: %d", rslt);

}