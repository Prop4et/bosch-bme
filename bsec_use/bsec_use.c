#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../includes/bme68x/bme68x_defs.h"
#include "../includes/bme68x/bme68x.h"
#include "../includes/bme_api/bme68x_API.h"
#include "../includes/bsec/bsec_datatypes.h"
#include "../includes/bsec/bsec_interface.h"

int main() {

    /*
        BSEC VARIABLES
    */
    bsec_library_return_t rslt_bsec;
    //measurements basically
    bsec_sensor_configuration_t requested_virtual_sensors[5];
    uint8_t n_requested_virtual_sensors = 5;
    // Allocate a struct for the returned physical sensor settings
    bsec_sensor_configuration_t required_sensor_settings[
    BSEC_MAX_PHYSICAL_SENSOR]; //should i put 1 since i have no shuttle?
    uint8_t n_required_sensor_settings = BSEC_MAX_PHYSICAL_SENSOR;
    //configuration coming from bsec
    bsec_bme_settings_t conf_bsec;
    bsec_input_t input[4];
    uint8_t n_input = 4;
    bsec_output_t output[5];
    uint8_t n_output=5;
    /*
        BME API VARIABLES
    */
    struct bme68x_dev bme;
    struct bme68x_data data;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    uint16_t sample_count = 1;
    int8_t rslt_api;
    stdio_init_all();
    
    sleep_ms(10000);
    /*
        INITIALIZATION BSEC LIBRARY
    */

    printf("...initialization BSEC\n");
    rslt_bsec = bsec_init();
    check_rslt_bsec(rslt_bsec, "BSEC_INIT"); 

    requested_virtual_sensors[0].sensor_id = BSEC_OUTPUT_IAQ;
    requested_virtual_sensors[0].sample_rate = BSEC_SAMPLE_RATE_LP;
    requested_virtual_sensors[1].sensor_id = BSEC_OUTPUT_RAW_TEMPERATURE;
    requested_virtual_sensors[1].sample_rate = BSEC_SAMPLE_RATE_LP;
    requested_virtual_sensors[2].sensor_id = BSEC_OUTPUT_RAW_PRESSURE;
    requested_virtual_sensors[2].sample_rate = BSEC_SAMPLE_RATE_LP;
    requested_virtual_sensors[3].sensor_id = BSEC_OUTPUT_RAW_HUMIDITY;
    requested_virtual_sensors[3].sample_rate = BSEC_SAMPLE_RATE_LP;
    requested_virtual_sensors[4].sensor_id = BSEC_OUTPUT_CO2_EQUIVALENT;
    requested_virtual_sensors[4].sample_rate = BSEC_SAMPLE_RATE_LP;
    

    // Call bsec_update_subscription() to enable/disable the requested virtual sensors
    rslt_bsec = bsec_update_subscription(requested_virtual_sensors, n_requested_virtual_sensors,
    required_sensor_settings, &n_required_sensor_settings);
    check_rslt_bsec(rslt_bsec, "BSEC_UPDATE_SUBSCRIPTION");

    rslt_bsec = bsec_sensor_control(time_us_64()*1000, &conf_bsec);
    check_rslt_bsec(rslt_bsec, "BSEC_SENSOR_CONTROL");

    /*
        INITIALIZATION BME CONFIGURATINO
    */
    printf("...initialization BME688\n");
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
    rslt_api = bme68x_init(&bme);
    check_rslt_api(rslt_api, "INIT");

    /*
        Set oversampling for measurements
    */

    conf.os_hum = conf_bsec.humidity_oversampling;
    conf.os_pres = conf_bsec.pressure_oversampling;
    conf.os_temp = conf_bsec.temperature_oversampling;
    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE;

    /*  
        Set the remaining gas sensor settings and link the heating profile 
        enable the heater plate
    */

    heatr_conf.enable = conf_bsec.run_gas;
    heatr_conf.heatr_temp = conf_bsec.heater_temperature;
    heatr_conf.heatr_dur = conf_bsec.heater_duration;

    rslt_api = bme68x_set_conf(&conf, &bme);
    check_rslt_api(rslt_api, "bme68x_set_conf");

    rslt_api = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr_conf, &bme);
    check_rslt_api(rslt_api, "bme68x_set_heatr_conf");

    uint8_t n_fields;
    uint32_t del_period;
    uint64_t time_us;
    while(1){
        rslt_api = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme); 
        check_rslt_api(rslt_api, "bme68x_set_op_mode");

        del_period = bme68x_get_meas_dur(BME68X_FORCED_MODE, &conf, &bme) + (heatr_conf.heatr_dur * 1000);

        bme.delay_us(del_period, bme.intf_ptr);
        time_us = time_us_64()*1000;
        rslt_api = bme68x_get_data(BME68X_FORCED_MODE, &data, &n_fields, &bme);
        check_rslt_api(rslt_api, "bme68x_get_data");
        
        if (rslt_api == BME68X_OK){
            if (! ((data.idac != 0x00) && (data.idac != 0xFF) ||
                (data.status & BME68X_GASM_VALID_MSK))){
                check_rslt_api(BME68X_E_SELF_TEST, "bme68x_get_data_mask");
            }
        }
        input[0].sensor_id = BSEC_INPUT_GASRESISTOR;
        input[0].signal = data.gas_resistance;
        input[0].time_stamp= time_us;
        input[1].sensor_id = BSEC_INPUT_TEMPERATURE;
        input[1].signal = data.temperature;
        input[1].time_stamp= time_us;
        input[2].sensor_id = BSEC_INPUT_HUMIDITY;
        input[2].signal = data.humidity;
        input[2].time_stamp= time_us;
        input[3].sensor_id = BSEC_INPUT_PRESSURE;
        input[3].signal = data.pressure;
        input[3].time_stamp = time_us;

        rslt_bsec = bsec_do_steps(input, n_input, output, &n_output);

        if(rslt_bsec == BSEC_OK){
            for(int i = 0; i < n_output; i++){
                switch(output[i].sensor_id){
                    case BSEC_OUTPUT_IAQ:
                        printf("IAQ\t| Accuracy\n");
                        printf("%.2f\t| %d \n", output[i].signal, output[i].accuracy);
                        break;
                    case BSEC_OUTPUT_STATIC_IAQ:
                        printf("STATIC IAQ\n");
                        break;
                    case BSEC_OUTPUT_CO2_EQUIVALENT:
                        printf("CO2[ppm]\t| Accuracy\n");
                        printf("%.2f\t| %d \n", output[i].signal, output[i].accuracy);
                        break;
                    case BSEC_OUTPUT_RAW_TEMPERATURE:
                            printf("Temperature[°C]\t| Accuracy\n");
                            printf("%.2f\t| %d \n", output[i].signal, output[i].accuracy);
                        break;
                    case BSEC_OUTPUT_RAW_HUMIDITY:
                            printf("Humidity[%%rH]\t| Accuracy\n");
                            printf("%.2f\t| %d \n", output[i].signal, output[i].accuracy);
                        break;
                    case BSEC_OUTPUT_RAW_PRESSURE:
                            printf("Pressure[atm]\t| Accuracy\n");
                            printf("%.2f\t| %d \n", output[i].signal/101325, output[i].accuracy);
                        break;
                    case BSEC_OUTPUT_RAW_GAS:
                        printf("[Ohm]\t| Accuracy\n");
                        printf("%.2f\t| %d \n", output[i].signal, output[i].accuracy);
                        break;
                    case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
                        printf("VOC\n");
                        break;
                    }
            }
            printf("\n");
        }

        //sensor goes automatically to sleep
        //it means i should save the configuration somewhere and load it back probably
        sleep_ms(5000);
    }

}