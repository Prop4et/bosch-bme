# bosch-bme
pico integration with bosch bme sensor with different examples 

The worflow to make everything work consists in installing the toolchain first, following the getting started with Pico [guide](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) provided by Raspberry.   
If working on Linux:
- everything works fine, you may want to add the export command to your /etc/profile (and all the other files if needed) to permanently create the PICO_SDK_PATH environment variable and avoid exporting that path everytime a new terminal is opened
Working on Windows with VSC:
- launch VSC from a VS command line, that helps in setting up all the environmental variables
- set "NMake Makefiles" as the preferred generator for cmake, this generator is usable only if MSVC is installed by VS installed and if VSC is launched through the VS cl.

After everything is set up, pulling the repository inside a folder should be enough to have everything set up for the cmake to compile the files and create the executables.
All the executables are created inside a build folder located in the root folder. Inside the build folder there will be, between all things, the name of the executables as folder, in which the .uf2 files are located (if built) that can be passed to the pico.
By default all the executables are set to use USB for the output and not UART. To read the outputs of the Pico it is possible:
- on Windows, you can use Putty to connect to the port (COMn) on which the Pico is connected. To discover the port it's enough to check the serial devices under device handling.
- on Linux, you can launch screen from the cl with the port (/dev/ttyACMn or /dev/ttyUSBn) on which the Pico is connected. To discover the port it should be enough to run ls /dev/ttyACM* /dev/ttyUSB* and take the result if the pico is the only serial device connected.
The baud rate for a Pico is 115200
