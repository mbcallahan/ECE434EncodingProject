#!/bin/bash
#configure pins
config-pin P9_11 uart
config-pin P9_13 uart
config-pin P8_38 uart
config-pin P8_37 uart

stty -F /dev/ttyS4 9600 raw -echo
stty -F /dev/ttyS5 9600 raw -echo
