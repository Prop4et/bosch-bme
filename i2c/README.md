## i2c
this folder contains:
- the CMakeLists.txt used to compile the executable with cmake
- i2c.c that is the executable
--- ---
i2c.c is a little piece of software that checks if the I2C bus is working by requesting the <em>DEVICE ID</em> of the sensor.  
If the id received is the same as the one specified by BOSCH then the communication was successful, otherwise the bus needs to be checked 