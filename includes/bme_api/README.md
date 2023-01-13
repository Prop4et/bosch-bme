## bme_api
this folder contains:
- the CMakeLists.txt used to create the library with cmake
- bme68x_API.h that is the header file
- bme68x_API.c that is the source file
--- ---
The purpose of this library is to:
1. define I2C pins
2. implement the communication functions for I2C
3. implement the function assigned to the bme68x_dev structure during the initialization
4. implement utility functions like checking operations results

The source file also presents:
- declaration of I2C tpye
- dev_addr variable to store the hex value of the I2C bus address