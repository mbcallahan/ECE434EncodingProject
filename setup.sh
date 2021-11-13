#!/bin/bash
#configure pins
config-pin P9_11 uart
config-pin P9_13 uart
config-pin P8_38 uart
config-pin P8_37 uart

stty -F /dev/ttyS4 115200 raw -echo
stty -F /dev/ttyS5 115200 raw -echo

sudo insmod encode.ko
sudo insmod decode.ko

sudo chmod 666 /dev/UARTdecode
sudo chmod 666 /dev/UARTencode
