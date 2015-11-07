#! /usr/bin/python

# To change this license header, choose License Headers in Project Properties.
# To change this template file, choose Tools | Templates
# and open the template in the editor.

__author__ = "hev"
__date__ = "$27-09-2015 18:06:53$"

import serial
import time
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=5.0) # this will reset Arduino
time.sleep(2) # therefore we will wait a little.
ser.write('I00220500000101')
time.sleep(1)
print ser.read(5)
