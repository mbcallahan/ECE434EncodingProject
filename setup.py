#!/usr/bin/python3

#this initializes UARTs 4 and 5. this can be used to check connection with minicom. I will copy over the uEnv file needed to get this to work as well. 
import Adafruit_BBIO.UART as UART


UART.setup("UART4");
UART.setup("UART5");
