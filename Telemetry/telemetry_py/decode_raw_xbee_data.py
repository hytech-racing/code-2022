import sys
import datetime
import binascii
from cobs import cobs
import codecs
import struct

def main():
    if len(sys.argv) != 2:
        print('Usage:')
        print('debug.py <raw xbee file>')
        quit()
    countGood = 0
    countBad = 0
    f = open(sys.argv[1])
    for line in f.readlines():
        for frame in line.split("00 "):
            msg = decode(frame)
            if (msg != -1):
                countGood += 1
                id = ord(msg[0])
                print("CAN ID:        0x" + binascii.hexlify(msg[0]).upper())
                size = ord(msg[4])
                print("MSG LEN:       " + str(size))
                if (id == 0xA0):
                    print("MODULE A TEMP: " + str(b2i16(msg[5:7]) / 10.) + " C")
                    print("MODULE B TEMP: " + str(b2i16(msg[7:9]) / 10.) + " C")
                    print("MODULE C TEMP: " + str(b2i16(msg[9:11]) / 10.) + " C")
                    print("GATE DRIVER BOARD TEMP: " + str(b2i16(msg[11:13]) / 10.) + " C")
                if (id == 0xA2):
                    print("RTD 4 TEMP: " + str(b2i16(msg[5:7]) / 10.) + " C")
                    print("RTD 5 TEMP: " + str(b2i16(msg[7:9]) / 10.) + " C")
                    print("MOTOR TEMP: " + str(b2i16(msg[9:11]) / 10.) + " C")
                    print("TORQUE SHUDDER: " + str(b2i16(msg[11:13]) / 10.) + " Nm")
                if (id == 0xA5):
                    print("MOTOR ANGLE: " + str(b2i16(msg[5:7]) / 10.))
                    print("MOTOR SPEED: " + str(b2i16(msg[7:9])) + " RPM")
                    print("ELEC OUTPUT FREQ: " + str(b2i16(msg[9:11]) / 10.))
                    print("DELTA RESOLVER FILT: " + str(b2i16(msg[11:13])))
                if (id == 0xA6):
                    print("PHASE A CURRENT: " + str(b2i16(msg[5:7]) / 10.) + " A")
                    print("PHASE B CURRENT: " + str(b2i16(msg[7:9]) / 10.) + " A")
                    print("PHASE C CURRENT: " + str(b2i16(msg[9:11]) / 10.) + " A")
                    print("DC BUS CURRENT: " + str(b2i16(msg[11:13]) / 10.) + " A")
                if (id == 0xA7):
                    print("DC BUS VOLTAGE: " + str(b2i16(msg[5:7]) / 10.) + " V")
                    print("OUTPUT VOLTAGE: " + str(b2i16(msg[7:9]) / 10.) + " V")
                    print("PHASE AB VOLTAGE: " + str(b2i16(msg[9:11]) / 10.) + " V")
                    print("PHASE BC VOLTAGE: " + str(b2i16(msg[11:13]) / 10.) + " V")
                if (id == 0xAA):
                    print("VSM STATE: " + str(b2ui16(msg[5:7])))
                    print("INVERTER STATE: " + str(ord(msg[7])))
                    print("INVERTER RUN MODE: " + str(ord(msg[9]) & 0x1))
                    print("INVERTER ACTIVE DISCHARGE STATE: " + str((ord(msg[9]) & 0xE0) >> 5))
                    print("INVERTER COMMAND MODE: " + str(ord(msg[10])))
                    print("INVERTER ENABLE: " + str(ord(msg[11]) & 0x1))
                    print("INVERTER LOCKOUT: " + str((ord(msg[11]) & 0x80) >> 7))
                    print("DIRECTION COMMAND: " + str(ord(msg[12])))
                if (id == 0xAB):
                    print("POST FAULT LO: 0x" + binascii.hexlify(msg[6]).upper() + binascii.hexlify(msg[5]).upper())
                    print("POST FAULT HI: 0x" + binascii.hexlify(msg[8]).upper() + binascii.hexlify(msg[7]).upper())
                    print("RUN FAULT LO: 0x" + binascii.hexlify(msg[10]).upper() + binascii.hexlify(msg[9]).upper())
                    print("RUN FAULT HI: 0x" + binascii.hexlify(msg[12]).upper() + binascii.hexlify(msg[11]).upper())
                if (id == 0xAC):
                    print("COMMANDED TORQUE: " + str(b2i16(msg[5:7]) / 10.) + " Nm")
                    print("TORQUE FEEDBACK: " + str(b2i16(msg[7:9]) / 10.) + " Nm")
                    print("RMS UPTIME: " + str(int(b2ui32(msg[9:13]) * .003)) + " s")
                if (id == 0xC0):
                    print("FCU REQUESTED TORQUE: " + str(b2i16(msg[5:7]) / 10.) + " N")
                    #print("FCU REQUESTED INVERTER ENABLE: " + str(ord(msg[10]) & 0x1))
                if (id == 0xD0):
                    print("RCU STATE: " + str(ord(msg[5])))
                    print("RCU FLAGS: 0x" + binascii.hexlify(msg[6]).upper())
                    print("GLV BATT VOLTAGE: " + str(b2ui16(msg[7:9]) / 100.) + " V")
                    print("RCU BMS FAULT: " + str(not ord(msg[6]) & 0x1))
                    print("RCU IMD FAULT: " + str(not (ord(msg[6]) & 0x2) >> 1))
                if (id == 0xD2):
                    print("FCU STATE: " + str(ord(msg[5])))
                    print("FCU FLAGS: 0x" + binascii.hexlify(msg[6]).upper())
                    print("FCU START BUTTON ID: " + str(ord(msg[7])))
                    print("FCU BRAKE ACT: " + str((ord(msg[6]) & 0x8) >> 3))
                    print("FCU IMPLAUS ACCEL: " + str(ord(msg[6]) & 0x1))
                    print("FCU IMPLAUS BRAKE: " + str((ord(msg[6]) & 0x4) >> 2))
                if (id == 0xD3):
                    print("FCU PEDAL ACCEL 1: " + str(b2ui16(msg[5:7])))
                    print("FCU PEDAL ACCEL 2: " + str(b2ui16(msg[7:9])))
                    print("FCU PEDAL BRAKE: " + str(b2ui16(msg[9:11])))
                if (id == 0xD7):
                    print("BMS VOLTAGE AVERAGE: " + str(b2ui16(msg[5:7]) / 10e3) + " V")
                    print("BMS VOLTAGE LOW: " + str(b2ui16(msg[7:9]) / 10e3) + " V")
                    print("BMS VOLTAGE HIGH: " + str(b2ui16(msg[9:11]) / 10e3) + " V")
                    print("BMS VOLTAGE TOTAL: " + str(b2ui16(msg[11:13]) / 100.) + " V")
                if (id == 0xD9):
                    print("BMS AVERAGE TEMPERATURE: " + str(b2i16(msg[5:7]) / 100.) + " C")
                    print("BMS LOW TEMPERATURE: " + str(b2i16(msg[7:9]) / 100.) + " C")
                    print("BMS HIGH TEMPERATURE: " + str(b2i16(msg[9:11]) / 100.) + " C")
                if (id == 0xDB):
                    print("BMS STATE: " + str(ord(msg[5])))
                    print("BMS ERROR FLAGS: 0x" + binascii.hexlify(msg[7]).upper() + binascii.hexlify(msg[6]).upper())
                    print("BMS CURRENT: " + str(b2i16(msg[8:10]) / 100.) + " A")
            else:
                    countBad += 1
    f.close()
    print("Processed " + str(countGood) + " messages")
    print("Failed to process " + str(countBad) + " messages")

def b2i8(data):
    return struct.unpack("<1b", chr(ord(data[0])))[0]

def b2i16(data):
    return struct.unpack("<1h", chr(ord(data[0])) + chr(ord(data[1])))[0]

def b2ui16(data):
    return struct.unpack("<1H", chr(ord(data[0])) + chr(ord(data[1])))[0]

def b2ui32(data):
    return struct.unpack("<1I", chr(ord(data[0])) + chr(ord(data[1])) + chr(ord(data[2])) + chr(ord(data[3])))[0]

def decode(frame):
    print("----------------")
    frame = ''.join(char for char in frame if char.isalnum())
    if (len(frame) != 32):
        print("Malformed frame len " + str(len(frame)) + " encountered - skipping")
        return -1
    frameprint = ''
    odd = False
    for char in frame:
        frameprint += char
        if odd:
            frameprint += " "
        odd = not odd
    print("Encoded frame: " + frameprint.upper())
    try:
        decoded = cobs.decode(binascii.unhexlify(frame.strip("\n\r")))
    except Exception as e:
        print("Decode failed: " + str(e))
        return -1
    # Calculate checksum
    checksum = fletcher16(decoded[0:13])
    cs_calc = binascii.hexlify(chr(checksum >> 8)).upper() + " " + binascii.hexlify(chr(checksum & 0xFF)).upper()
    cs_rcvd = binascii.hexlify(decoded[14]).upper() + " " + binascii.hexlify(decoded[13]).upper()
    if cs_calc != cs_rcvd:
        print("Decode failed: Checksum mismatch - calc: " + cs_calc + " - rcvd: " + cs_rcvd)
        return -1
    out = "Decoded frame: "
    for char in decoded:
        out += binascii.hexlify(char).upper() + " "
    print(out)
    return decoded

def fletcher16(data):
    d = map(ord,data)
    index = 0
    c0 = 0
    c1 = 0
    i = 0
    length = len(d)
    while length >= 5802:
        for i in range(5802):
            c0 += d[index]
            c1 += c0
            index += 1
        c0 %= 255
        c1 %= 255
        length -= 5802
    
    index = 0
    for i in range(len(data)):
        c0 += d[index]
        c1 += c0
        index += 1
    c0 %= 255
    c1 %= 255
    return (c1 << 8 | c0)

main()