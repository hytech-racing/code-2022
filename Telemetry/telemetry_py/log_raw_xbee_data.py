import serial
import sys
import datetime
import binascii

def main():
    if len(sys.argv) != 2:
        print('Usage:')
        print('debug.py <serial device>')
        quit()
    filename = str(datetime.datetime.now()) + ".txt"
    ser = serial.Serial(sys.argv[1], 115200)
    while (True):
        data = ser.read()
        if len(data) > 0:
            with open(filename, "a") as f:
                f.write(binascii.hexlify(data) + ' ')

main()
