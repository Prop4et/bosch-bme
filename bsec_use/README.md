# bsec_use
this folder contains:
- the CMakeLists.txt used to compile the executable with cmake
- bsec_use.c that is the executable
--- ---
The bsec_use.c file is continuosly evolving with new functions to finally get to a point where it could be considered complete with a full set of functionalities.  
The main workflow of the executable is:
1. including the libraries to work with the BME68x API and the BSEC API
2. initialize the structures used by the BSEC API
3. initialize the structure used by the BME68x API
4. check for I2C comms
5. configuration of the settings for the actual readings
6. closed loop to
    - read the raw values from the sensor
    - pass the value to the BSEC API to get the derived values
    - print the values (change to use the values as pleased)

SPI protocol is not implemented yet  

The executable uses the BSEC library for the CortexM0+ processor that is the processor used by the RP2040 chip on the pico