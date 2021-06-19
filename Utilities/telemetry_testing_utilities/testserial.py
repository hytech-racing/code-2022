#!/usr/bin/python

import time
from serial import Serial
#initialization and open the port
import binascii

#possible timeout values:
#    1. None: wait forever, block call
#    2. 0: non-blocking mode, return immediately
#    3. x, x is bigger than 0, float allowed, timeout block call

ser = Serial()
ser.port = "/dev/cu.usbserial-00001014"
ser.baudrate = 115200

try: 
    ser.open()
except Exception as e:
    print("error open serial port:", str(e))
    exit()

if ser.isOpen():

    try:
        ser.flushInput() #flush input buffer, discarding all its contents
        ser.flushOutput()#flush output buffer, aborting current output 
                    #and discard all that is in buffer

            #write data

        while True:
            message = open('message.txt', 'rb') 
            ser.write(message.readline())
            
            if ser.inWaiting() > 0:
                response = ser.read(ser.inWaiting())
                print(response)
            time.sleep(0.1)
            # response = ser.readline()
            # print(response)

        #time.sleep(0.05)  #give the serial port sometime to receive the data


        ser.close()
    except Exception as e:
        print("error communicating...:", str(e))

else:
    print("cannot open serial port")
