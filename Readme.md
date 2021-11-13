#Encoding Module Project
This project implements kernel modules that encode and decode messages to be sent over UARTs 4 and 5 on the beagle-bone black.

##Setup
First, to enable the UART devices, you need to go into /boot/uEnv.txt and disable the video and audio drivers to free up the UART devices. Then connect UART4 MISO to UART 5 MOSI and UART4 MOSI to UART 5 MISO. This equates to P9_13 to P8_38 and P9_11 to P8_37

You need to setup your enviornment to be able to compile modules. Then you need to make the modules in this repository, and run setup.sh to configure the UART devices and insert the kernel modules. 
