#include <stdio.h>

const char* state_file_name = "stat_file";
const char* config_file_name = "config_file"; 

void print_results(int id, float signal, int accuracy);

void check_fs_error(int rslt, char msg[]);