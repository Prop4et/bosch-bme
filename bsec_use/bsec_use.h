#include <stdio.h>
#include "../includes/bme68x/bme68x_defs.h"

const char* state_file_name = "stat_file";
const char* config_file_name = "config_file"; 

void print_raw_results(struct bme68x_data data);
void print_results(int id, float signal, int accuracy);
void check_fs_error(int rslt, char msg[]);