#include "bsec_use.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../includes/bme68x/bme68x_defs.h"
#include "../includes/bme68x/bme68x.h"
#include "../includes/bme_api/bme68x_API.h"
#include "../includes/bsec/bsec_datatypes.h"
#include "../includes/bsec/bsec_interface.h"
#include "../includes/littlefs-lib/pico_hal.h"


int main() {
    /*
        FILESYSTEM VARIABLES
    */
    lfs_size_t rslt_fs;
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
    //state handling
    uint8_t serialized_state[BSEC_MAX_STATE_BLOB_SIZE];
    uint32_t n_serialized_state_max = BSEC_MAX_STATE_BLOB_SIZE;
    uint32_t n_serialized_state = BSEC_MAX_STATE_BLOB_SIZE;
    uint8_t work_buffer_state[BSEC_MAX_WORKBUFFER_SIZE];
    uint32_t n_work_buffer_size = BSEC_MAX_WORKBUFFER_SIZE;
    //configuration on shut down
    uint8_t serialized_settings[BSEC_MAX_PROPERTY_BLOB_SIZE];
    uint32_t n_serialized_settings_max = BSEC_MAX_PROPERTY_BLOB_SIZE;
    uint8_t work_buffer[BSEC_MAX_WORKBUFFER_SIZE];
    uint32_t n_work_buffer = BSEC_MAX_WORKBUFFER_SIZE;
    uint32_t n_serialized_settings = 0;
    //
    uint64_t time_us;
    /*
        BME API VARIABLES
    */
    struct bme68x_dev bme;
    struct bme68x_data data;
    struct bme68x_conf conf;
    struct bme68x_heatr_conf heatr_conf;
    uint16_t sample_count = 1;
    int8_t rslt_api;
    uint8_t n_fields;
    uint32_t del_period;

    /*
        OTHER VARIABLES
    */
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    stdio_init_all();
    sleep_ms(10000);
    /*
        little FS set up, it mounts a file system on the flash memory to save the state file
        if set to true it formats everything
        if set to false it keeps the files as they were
    */
    printf("...mounting FS\n");
    if (pico_mount(true) != LFS_ERR_OK) {
        printf("Error mounting FS\n");
    }

    printf("...initialization BSEC\n");
    rslt_bsec = bsec_init();
    check_rslt_bsec(rslt_bsec, "BSEC_INIT"); 
    /*
        INITIALIZATION BSEC LIBRARY
    */
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
    
    //read state to get the previous state and avoid restarting everything
    int state_file = pico_open(state_file_name, LFS_O_CREAT | LFS_O_RDWR);
    check_fs_error(state_file, "Error opening state file"); 

    rslt_fs = pico_read(state_file, serialized_state, BSEC_MAX_PROPERTY_BLOB_SIZE*sizeof(uint8_t));
    check_fs_error(state_file, "Error while reading state file");  
    
    //if there is a state file saved somewhere it loads the variable back
    if(rslt_fs > 0){
        printf("...resuming the state\n");
        rslt_bsec = bsec_set_state(serialized_state, n_serialized_state, work_buffer_state, n_work_buffer_size);
        check_rslt_bsec(rslt_bsec, "BSEC_SET_STATE");
    }
    
    rslt_fs = pico_rewind(state_file);
    check_fs_error(state_file, "Error while rewinding state file");

    // Call bsec_update_subscription() to enable/disable the requested virtual sensors
    rslt_bsec = bsec_update_subscription(requested_virtual_sensors, n_requested_virtual_sensors,
    required_sensor_settings, &n_required_sensor_settings);
    check_rslt_bsec(rslt_bsec, "BSEC_UPDATE_SUBSCRIPTION");

    rslt_bsec = bsec_sensor_control(time_us_64()*1000, &conf_bsec);
    check_rslt_bsec(rslt_bsec, "BSEC_SENSOR_CONTROL");

    /*
        INITIALIZATION BME CONFIGURATION
    */
    printf("...initialization BME688\n");
    bme_interface_init(&bme, BME68X_I2C_INTF);
    uint8_t data_id;

    //read device id after init to see if everything works for now and to check that the device communicates with I2C
    bme.read(BME68X_REG_CHIP_ID, &data_id, 1, bme.intf_ptr);
    if(data_id != BME68X_CHIP_ID){
        printf("Cannot communicate with BME688\n");
        printf("CHIP_ID: %x\t ID_READ: %x\n", BME68X_CHIP_ID, data_id);
    }
    else{
        bme.chip_id = data_id;
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

    uint16_t loops = 30;
    while(loops >= 0){
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
        /*
            input variables for the BSEC API
        */
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
                print_results(output[i].sensor_id, output[i].signal, output[i].accuracy);
            }
            printf("\n");
        }
        loops -= 1;
        //sensor goes automatically to sleep
        sleep_ms(10000);
    }

    rslt_bsec = bsec_get_state(0, serialized_state, n_serialized_state_max, work_buffer_state, n_work_buffer_size, &n_serialized_state);
    check_rslt_bsec(rslt_bsec, "BSEC_GET_STATE");

    rslt_fs = pico_write(state_file, serialized_state, BSEC_MAX_PROPERTY_BLOB_SIZE*sizeof(uint8_t));
    check_fs_error(rslt_fs, "Error writing the file");
    pico_fflush(state_file);
	
    int pos = pico_lseek(state_file, 0, LFS_SEEK_CUR);
    printf("Written %d byte for file %s\n", pos, state_file);

    rslt_fs = pico_close(state_file);
    check_fs_error(rslt_fs, "Error closing the file");

    sleep_ms(1000);
    pico_unmount();
    gpio_put(PICO_DEFAULT_LED_PIN, 1);

    /*rslt_bsec = bsec_get_configuration(0, serialized_settings, n_serialized_settings_max, work_buffer, n_work_buffer, &n_serialized_settings);
    check_rslt_bsce(rslt_bsec, "BSEC_GET_CONFIGURATION"); */  

}

//utility to print results
void print_results(int id, float signal, int accuracy){
    switch(id){
        case BSEC_OUTPUT_IAQ:
            printf("IAQ\t | Accuracy\n");
            printf("%.2f\t | %d \n", signal, accuracy);
            break;
        case BSEC_OUTPUT_STATIC_IAQ:
            printf("STATIC IAQ\n");
            break;
        case BSEC_OUTPUT_CO2_EQUIVALENT:
            printf("CO2[ppm]\t | Accuracy\n");
            printf("%.2f\t | %d \n", signal, accuracy);
            break;
        case BSEC_OUTPUT_RAW_TEMPERATURE:
                printf("Temperature[°C]\t | Accuracy\n");
                printf("%.2f\t | %d \n", signal, accuracy);
            break;
        case BSEC_OUTPUT_RAW_HUMIDITY:
                printf("Humidity[%%rH]\t | Accuracy\n");
                printf("%.2f\t | %d \n", signal, accuracy);
            break;
        case BSEC_OUTPUT_RAW_PRESSURE:
                printf("Pressure[atm]\t | Accuracy\n");
                printf("%.2f\t | %d \n", signal/101325, accuracy);
            break;
        case BSEC_OUTPUT_RAW_GAS:
            printf("[Ohm]\t | Accuracy\n");
            printf("%.2f\t | %d \n", signal, accuracy);
            break;
        case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
            printf("VOC\n");
            break;
    }
}

void check_fs_error(int rslt, char msg[]){
    if(rslt < 0){
        printf("FS error %s\n", msg);
        blink();
    }
}