import curses
import sys
import datetime
import binascii
from cobs import cobs
import codecs
import struct
import paho.mqtt.client as mqtt

screen = curses.initscr()
countGoodFrames = 0
countBadFrames = 0

# Set up filenames for logging
timestamp = str(datetime.datetime.now())
filename = timestamp + ".txt"
filenameRaw = "raw " + timestamp + ".csv"

def mqtt_connect(client, userdata, flags, rc):
    client.subscribe("hytech_car/telemetry")
    screen.addstr(0,47,' - CONNECTED')
    client.publish("hytech_car/telemetry", "Python client connected")

def mqtt_message(client, userdata, msg):
    screen.addstr(0,59,' - RECEIVED')
    global countGoodFrames
    global countBadFrames

    # TODO check format of incoming message for errors
    timestamp = msg.payload[0:msg.payload.find(b',')]
    frame = msg.payload[msg.payload.find(b',') + 1:-1]
    frame = binascii.hexlify(frame)
    data = unpack(frame)

    if data != -1:
        with open(filenameRaw, "a") as f:
            size = ord(data[4])
            raw = binascii.hexlify(data[0]).upper() + "," + binascii.hexlify(data[5:5 + size]).upper()
            f.write(str(datetime.datetime.now()) + ',' + raw + "\n")
        countGoodFrames += 1
        lines = decode(data)
        with open(filename, "a") as f:
            # TODO update to use timestamp sent over LTE instead of local timestamp
            timestamp = str(datetime.datetime.now())
            for line in lines:
                updateScreen(line)
                f.write(timestamp + ' ' + line + '\n')
    else:
        countBadFrames += 1

def main():
    if len(sys.argv) != 1:
        print('Usage:')
        print('telemetry_lte_xbee.py')
        quit()
    screen.border('#', '#', '#', '#', 0, 0, 0, 0)
    screen.addstr(0,5,'HYTECH RACING 2019 VEHICLE SERIAL DEBUGGER')
    screen.addstr(3,5,'RMS INVERTER')
    screen.addstr(4,5,'MODULE A TEMP: ')
    screen.addstr(5,5,'MODULE B TEMP: ')
    screen.addstr(6,5,'MODULE C TEMP: ')
    screen.addstr(7,5,'GATE DRIVER BOARD TEMP: ')
    screen.addstr(8,5,'RTD 4 TEMP: ')
    screen.addstr(9,5,'RTD 5 TEMP: ')
    screen.addstr(10,5,'MOTOR TEMP: ')
    screen.addstr(11,5,'TORQUE SHUDDER: ')
    screen.addstr(12,5,'MOTOR ANGLE: ')
    screen.addstr(13,5,'MOTOR SPEED: ')
    screen.addstr(14,5,'ELEC OUTPUT FREQ: ')
    screen.addstr(15,5,'DELTA RESOLVER FILT: ')
    screen.addstr(16,5,'PHASE A CURRENT: ')
    screen.addstr(17,5,'PHASE B CURRENT: ')
    screen.addstr(18,5,'PHASE C CURRENT: ')
    screen.addstr(19,5,'DC BUS CURRENT: ')
    screen.addstr(20,5,'DC BUS VOLTAGE: ')
    screen.addstr(21,5,'OUTPUT VOLTAGE: ')
    screen.addstr(22,5,'PHASE AB VOLTAGE: ')
    screen.addstr(23,5,'PHASE BC VOLTAGE: ')
    screen.addstr(24,5,'VSM STATE: ')
    screen.addstr(25,5,'INVERTER STATE: ')
    screen.addstr(26,5,'INVERTER RUN MODE: ')
    screen.addstr(27,5,'INVERTER ACTIVE DISCHARGE STATE: ')
    screen.addstr(28,5,'INVERTER COMMAND MODE: ')
    screen.addstr(29,5,'INVERTER ENABLE: ')
    screen.addstr(30,5,'INVERTER LOCKOUT: ')
    screen.addstr(31,5,'DIRECTION COMMAND: ')
    screen.addstr(32,5,'POST FAULT LO: ')
    screen.addstr(33,5,'POST FAULT HI: ')
    screen.addstr(34,5,'RUN FAULT LO: ')
    screen.addstr(35,5,'RUN FAULT HI: ')
    screen.addstr(36,5,'COMMANDED TORQUE: ')
    screen.addstr(37,5,'TORQUE FEEDBACK: ')
    screen.addstr(38,5,'RMS UPTIME: ')

    screen.addstr(3,55,'BATTERY MANAGEMENT SYSTEM')
    screen.addstr(4,55,'BMS AVERAGE TEMPERATURE: ')
    screen.addstr(5,55,'BMS LOW TEMPERATURE: ')
    screen.addstr(6,55,'BMS HIGH TEMPERATURE: ')
    screen.addstr(7,55,'BMS STATE: ')
    screen.addstr(8,55,'BMS ERROR FLAGS: ')
    screen.addstr(9,55,'BMS CURRENT: ')
    screen.addstr(10,55,'BMS VOLTAGE AVERAGE: ')
    screen.addstr(11,55,'BMS VOLTAGE LOW: ')
    screen.addstr(12,55,'BMS VOLTAGE HIGH: ')
    screen.addstr(13,55,'BMS VOLTAGE TOTAL: ')

    screen.addstr(15,55,'FRONT CONTROL UNIT')
    screen.addstr(16,55,'FCU UPTIME: ')
    screen.addstr(17,55,'FCU STATE: ')
    screen.addstr(18,55,'START BUTTON ID: ')
    screen.addstr(19,55,'FCU RAW TORQUE: ')
    screen.addstr(20,55,'FCU REQUESTED TORQUE: ')
    screen.addstr(21,55,'FCU PEDAL ACCEL 1: ')
    screen.addstr(22,55,'FCU PEDAL ACCEL 2: ')
    screen.addstr(23,55,'FCU PEDAL BRAKE: ')
    screen.addstr(24,55,'FCU BRAKE ACT: ')
    screen.addstr(25,55,'FCU IMPLAUS ACCEL: ')
    screen.addstr(26,55,'FCU IMPLAUS BRAKE: ')

    screen.addstr(28,55,'REAR CONTROL UNIT')
    screen.addstr(29,55,'RCU UPTIME: ')
    screen.addstr(30,55,'RCU STATE: ')
    screen.addstr(31,55,'RCU FLAGS: ')
    screen.addstr(32,55,'GLV BATT VOLTAGE: ')
    screen.addstr(33,55,'RCU BMS FAULT: ')
    screen.addstr(34,55,'RCU IMD FAULT: ')

    screen.addstr(3,105,'BATTERY MANAGEMENT SYSTEM DETAILED VOLTAGES')
    screen.addstr(4,105,'IC 0 CELL 0: ')
    screen.addstr(5,105,'IC 0 CELL 1: ')
    screen.addstr(6,105,'IC 0 CELL 2: ')
    screen.addstr(7,105,'IC 0 CELL 3: ')
    screen.addstr(8,105,'IC 0 CELL 4: ')
    screen.addstr(9,105,'IC 0 CELL 5: ')
    screen.addstr(10,105,'IC 0 CELL 6: ')
    screen.addstr(11,105,'IC 0 CELL 7: ')
    screen.addstr(12,105,'IC 0 CELL 8: ')

    screen.addstr(14,105,'IC 1 CELL 0: ')
    screen.addstr(15,105,'IC 1 CELL 1: ')
    screen.addstr(16,105,'IC 1 CELL 2: ')
    screen.addstr(17,105,'IC 1 CELL 3: ')
    screen.addstr(18,105,'IC 1 CELL 4: ')
    screen.addstr(19,105,'IC 1 CELL 5: ')
    screen.addstr(20,105,'IC 1 CELL 6: ')
    screen.addstr(21,105,'IC 1 CELL 7: ')
    screen.addstr(22,105,'IC 1 CELL 8: ')

    screen.addstr(24,105,'IC 2 CELL 0: ')
    screen.addstr(25,105,'IC 2 CELL 1: ')
    screen.addstr(26,105,'IC 2 CELL 2: ')
    screen.addstr(27,105,'IC 2 CELL 3: ')
    screen.addstr(28,105,'IC 2 CELL 4: ')
    screen.addstr(29,105,'IC 2 CELL 5: ')
    screen.addstr(30,105,'IC 2 CELL 6: ')
    screen.addstr(31,105,'IC 2 CELL 7: ')
    screen.addstr(32,105,'IC 2 CELL 8: ')

    screen.addstr(34,105,'IC 3 CELL 0: ')
    screen.addstr(35,105,'IC 3 CELL 1: ')
    screen.addstr(36,105,'IC 3 CELL 2: ')
    screen.addstr(37,105,'IC 3 CELL 3: ')
    screen.addstr(38,105,'IC 3 CELL 4: ')
    screen.addstr(39,105,'IC 3 CELL 5: ')
    screen.addstr(40,105,'IC 3 CELL 6: ')
    screen.addstr(41,105,'IC 3 CELL 7: ')
    screen.addstr(42,105,'IC 3 CELL 8: ')

    screen.addstr(4,130,'IC 4 CELL 0: ')
    screen.addstr(5,130,'IC 4 CELL 1: ')
    screen.addstr(6,130,'IC 4 CELL 2: ')
    screen.addstr(7,130,'IC 4 CELL 3: ')
    screen.addstr(8,130,'IC 4 CELL 4: ')
    screen.addstr(9,130,'IC 4 CELL 5: ')
    screen.addstr(10,130,'IC 4 CELL 6: ')
    screen.addstr(11,130,'IC 4 CELL 7: ')
    screen.addstr(12,130,'IC 4 CELL 8: ')

    screen.addstr(14,130,'IC 5 CELL 0: ')
    screen.addstr(15,130,'IC 5 CELL 1: ')
    screen.addstr(16,130,'IC 5 CELL 2: ')
    screen.addstr(17,130,'IC 5 CELL 3: ')
    screen.addstr(18,130,'IC 5 CELL 4: ')
    screen.addstr(19,130,'IC 5 CELL 5: ')
    screen.addstr(20,130,'IC 5 CELL 6: ')
    screen.addstr(21,130,'IC 5 CELL 7: ')
    screen.addstr(22,130,'IC 5 CELL 8: ')

    screen.addstr(24,130,'IC 6 CELL 0: ')
    screen.addstr(25,130,'IC 6 CELL 1: ')
    screen.addstr(26,130,'IC 6 CELL 2: ')
    screen.addstr(27,130,'IC 6 CELL 3: ')
    screen.addstr(28,130,'IC 6 CELL 4: ')
    screen.addstr(29,130,'IC 6 CELL 5: ')
    screen.addstr(30,130,'IC 6 CELL 6: ')
    screen.addstr(31,130,'IC 6 CELL 7: ')
    screen.addstr(32,130,'IC 6 CELL 8: ')

    screen.addstr(34,130,'IC 7 CELL 0: ')
    screen.addstr(35,130,'IC 7 CELL 1: ')
    screen.addstr(36,130,'IC 7 CELL 2: ')
    screen.addstr(37,130,'IC 7 CELL 3: ')
    screen.addstr(38,130,'IC 7 CELL 4: ')
    screen.addstr(39,130,'IC 7 CELL 5: ')
    screen.addstr(40,130,'IC 7 CELL 6: ')
    screen.addstr(41,130,'IC 7 CELL 7: ')
    screen.addstr(42,130,'IC 7 CELL 8: ')

    screen.addstr(3,155,'BATTERY MANAGEMENT SYSTEM DETAILED TEMPERATURES')
    screen.addstr(4,155,'IC 0 THERM 0: ')
    screen.addstr(5,155,'IC 0 THERM 1: ')
    screen.addstr(6,155,'IC 0 THERM 2: ')

    screen.addstr(8,155,'IC 1 THERM 0: ')
    screen.addstr(9,155,'IC 1 THERM 1: ')
    screen.addstr(10,155,'IC 1 THERM 2: ')

    screen.addstr(12,155,'IC 2 THERM 0: ')
    screen.addstr(13,155,'IC 2 THERM 1: ')
    screen.addstr(14,155,'IC 2 THERM 2: ')

    screen.addstr(16,155,'IC 3 THERM 0: ')
    screen.addstr(17,155,'IC 3 THERM 1: ')
    screen.addstr(18,155,'IC 3 THERM 2: ')

    screen.addstr(4,180,'IC 4 THERM 0: ')
    screen.addstr(5,180,'IC 4 THERM 1: ')
    screen.addstr(6,180,'IC 4 THERM 2: ')

    screen.addstr(8,180,'IC 5 THERM 0: ')
    screen.addstr(9,180,'IC 5 THERM 1: ')
    screen.addstr(10,180,'IC 5 THERM 2: ')

    screen.addstr(12,180,'IC 6 THERM 0: ')
    screen.addstr(13,180,'IC 6 THERM 1: ')
    screen.addstr(14,180,'IC 6 THERM 2: ')

    screen.addstr(16,180,'IC 7 THERM 0: ')
    screen.addstr(17,180,'IC 7 THERM 1: ')
    screen.addstr(18,180,'IC 7 THERM 2: ')

    curses.wrapper(live)
    curses.endwin()

def live(screen):
    # Set up mqtt connection
    client = mqtt.Client()
    client.connect("hytech-telemetry.ryangallaway.me", 1883, 60)
    client.on_connect = mqtt_connect
    client.on_message = mqtt_message
    client.loop_start()
    
    screen.nodelay(True)

    # Write CSV Header
    with open(filenameRaw, "a") as f:
        f.write("timestamp,CAN ID,msg\n")
    
    # Wait for q to quit
    char = screen.getch()
    while char != ord('q') and char != ord('Q'):
        char = screen.getch()

    # Time to quit, disconnect MQTT
    client.loop_stop()
    client.disconnect() # TODO unsure if this should be called

    # Write out statistics
    with open(filename, "a") as f:
        f.write("Processed " + str(countGoodFrames) + " good frames - ignored " + str(countBadFrames) + " bad frames")

def updateScreen(incomingLine):
    if ('MODULE A TEMP' in incomingLine):
        clearLine(4,5)
        screen.addstr(4,5,incomingLine)
    if ('MODULE B TEMP' in incomingLine):
        clearLine(5,5)
        screen.addstr(5,5,incomingLine)
    if ('MODULE C TEMP' in incomingLine):
        clearLine(6,5)
        screen.addstr(6,5,incomingLine)
    if ('GATE DRIVER BOARD TEMP' in incomingLine):
        clearLine(7,5)
        screen.addstr(7,5,incomingLine)
    if ('RTD 4 TEMP' in incomingLine):
        clearLine(8,5)
        screen.addstr(8,5,incomingLine)
    if ('RTD 5 TEMP' in incomingLine):
        clearLine(9,5)
        screen.addstr(9,5,incomingLine)
    if ('MOTOR TEMP' in incomingLine):
        clearLine(10,5)
        screen.addstr(10,5,incomingLine)
    if ('TORQUE SHUDDER' in incomingLine):
        clearLine(11,5)
        screen.addstr(11,5,incomingLine)
    if ('MOTOR ANGLE' in incomingLine):
        clearLine(12,5)
        screen.addstr(12,5,incomingLine)
    if ('MOTOR SPEED' in incomingLine):
        clearLine(13,5)
        screen.addstr(13,5,incomingLine)
    if ('ELEC OUTPUT FREQ' in incomingLine):
        clearLine(14,5)
        screen.addstr(14,5,incomingLine)
    if ('DELTA RESOLVER FILT' in incomingLine):
        clearLine(15,5)
        screen.addstr(15,5,incomingLine)
    if ('PHASE A CURRENT' in incomingLine):
        clearLine(16,5)
        screen.addstr(16,5,incomingLine)
    if ('PHASE B CURRENT' in incomingLine):
        clearLine(17,5)
        screen.addstr(17,5,incomingLine)
    if ('PHASE C CURRENT' in incomingLine):
        clearLine(18,5)
        screen.addstr(18,5,incomingLine)
    if ('DC BUS CURRENT' in incomingLine):
        clearLine(19,5)
        screen.addstr(19,5,incomingLine)
    if ('DC BUS VOLTAGE' in incomingLine):
        clearLine(20,5)
        screen.addstr(20,5,incomingLine)
    if ('OUTPUT VOLTAGE' in incomingLine):
        clearLine(21,5)
        screen.addstr(21,5,incomingLine)
    if ('PHASE AB VOLTAGE' in incomingLine):
        clearLine(22,5)
        screen.addstr(22,5,incomingLine)
    if ('PHASE BC VOLTAGE' in incomingLine):
        clearLine(23,5)
        screen.addstr(23,5,incomingLine)
    if ('VSM STATE' in incomingLine):
        clearLine(24,5)
        screen.addstr(24,5,incomingLine)
    if ('INVERTER STATE' in incomingLine):
        clearLine(25,5)
        screen.addstr(25,5,incomingLine)
    if ('INVERTER RUN MODE' in incomingLine):
        clearLine(26,5)
        screen.addstr(26,5,incomingLine)
    if ('INVERTER ACTIVE DISCHARGE STATE' in incomingLine):
        clearLine(27,5)
        screen.addstr(27,5,incomingLine)
    if ('INVERTER COMMAND MODE' in incomingLine):
        clearLine(28,5)
        screen.addstr(28,5,incomingLine)
    if ('INVERTER ENABLE' in incomingLine):
        clearLine(29,5)
        screen.addstr(29,5,incomingLine)
    if ('INVERTER LOCKOUT' in incomingLine):
        clearLine(30,5)
        screen.addstr(30,5,incomingLine)
    if ('DIRECTION COMMAND' in incomingLine):
        clearLine(31,5)
        screen.addstr(31,5,incomingLine)
    if ('POST FAULT LO' in incomingLine):
        clearLine(32,5)
        screen.addstr(32,5,incomingLine)
    if ('POST FAULT HI' in incomingLine):
        clearLine(33,5)
        screen.addstr(33,5,incomingLine)
    if ('RUN FAULT LO' in incomingLine):
        clearLine(34,5)
        screen.addstr(34,5,incomingLine)
    if ('RUN FAULT HI' in incomingLine):
        clearLine(35,5)
        screen.addstr(35,5,incomingLine)
    if ('COMMANDED TORQUE:' in incomingLine):
        clearLine(36,5)
        screen.addstr(36,5,incomingLine)
    if ('TORQUE FEEDBACK' in incomingLine):
        clearLine(37,5)
        screen.addstr(37,5,incomingLine)
    if ('RMS UPTIME' in incomingLine):
        clearLine(38,5)
        screen.addstr(38,5,incomingLine)
    if ('BMS AVERAGE TEMPERATURE' in incomingLine):
        clearLine(4,55)
        screen.addstr(4,55,incomingLine)
    if ('BMS LOW TEMPERATURE' in incomingLine):
        clearLine(5,55)
        screen.addstr(5,55,incomingLine)
    if ('BMS HIGH TEMPERATURE' in incomingLine):
        clearLine(6,55)
        screen.addstr(6,55,incomingLine)
    if ('BMS STATE' in incomingLine):
        clearLine(7,55)
        screen.addstr(7,55,incomingLine)
    if ('BMS ERROR FLAGS' in incomingLine):
        clearLine(8,55)
        screen.addstr(8,55,incomingLine)
    if ('BMS CURRENT' in incomingLine):
        clearLine(9,55)
        screen.addstr(9,55,incomingLine)
    if ('BMS VOLTAGE AVERAGE' in incomingLine):
        clearLine(10,55)
        screen.addstr(10,55,incomingLine)
    if ('BMS VOLTAGE LOW' in incomingLine):
        clearLine(11,55)
        screen.addstr(11,55,incomingLine)
    if ('BMS VOLTAGE HIGH' in incomingLine):
        clearLine(12,55)
        screen.addstr(12,55,incomingLine)
    if ('BMS VOLTAGE TOTAL' in incomingLine):
        clearLine(13,55)
        screen.addstr(13,55,incomingLine)
    if ('FCU UPTIME' in incomingLine):
        clearLine(16,55)
        screen.addstr(16,55,incomingLine)
    if ('FCU STATE' in incomingLine):
        clearLine(17,55)
        screen.addstr(17,55,incomingLine)
    if ('FCU START BUTTON ID' in incomingLine):
        clearLine(18,55)
        screen.addstr(18,55,incomingLine)
    if ('FCU RAW TORQUE' in incomingLine):
        clearLine(19,55)
        screen.addstr(19,55,incomingLine)
    if ('FCU REQUESTED TORQUE' in incomingLine):
        clearLine(20,55)
        screen.addstr(20,55,incomingLine)
    if ('FCU PEDAL ACCEL 1' in incomingLine):
        clearLine(21,55)
        screen.addstr(21,55,incomingLine)
    if ('FCU PEDAL ACCEL 2' in incomingLine):
        clearLine(22,55)
        screen.addstr(22,55,incomingLine)
    if ('FCU PEDAL BRAKE' in incomingLine):
        clearLine(23,55)
        screen.addstr(23,55,incomingLine)
    if ('FCU BRAKE ACT' in incomingLine):
        clearLine(24,55)
        screen.addstr(24,55,incomingLine)
    if ('FCU IMPLAUS ACCEL' in incomingLine):
        clearLine(25,55)
        screen.addstr(25,55,incomingLine)
    if ('FCU IMPLAUS BRAKE' in incomingLine):
        clearLine(26,55)
        screen.addstr(26,55,incomingLine)
    if ('RCU UPTIME' in incomingLine):
        clearLine(29,55)
        screen.addstr(29,55,incomingLine)
    if ('RCU STATE' in incomingLine):
        clearLine(30,55)
        screen.addstr(30,55,incomingLine)
    if ('RCU FLAGS' in incomingLine):
        clearLine(31,55)
        screen.addstr(31,55,incomingLine)
    if ('GLV BATT VOLTAGE' in incomingLine):
        clearLine(32,55)
        screen.addstr(32,55,incomingLine)
    if ('RCU BMS FAULT' in incomingLine):
        clearLine(33,55)
        screen.addstr(33,55,incomingLine)
    if ('RCU IMD FAULT' in incomingLine):
        clearLine(34,55)
        screen.addstr(34,55,incomingLine)
    if ('IC 0 C' in incomingLine):
        row = 4 + int(incomingLine[10])
        clearLineShort(row,105)
        screen.addstr(row,105,incomingLine)
    if ('IC 1 C' in incomingLine):
        row = 14 + int(incomingLine[10])
        clearLineShort(row,105)
        screen.addstr(row,105,incomingLine)
    if ('IC 2 C' in incomingLine):
        row = 24 + int(incomingLine[10])
        clearLineShort(row,105)
        screen.addstr(row,105,incomingLine)
    if ('IC 3 C' in incomingLine):
        row = 34 + int(incomingLine[10])
        clearLineShort(row,105)
        screen.addstr(row,105,incomingLine)
    if ('IC 4 C' in incomingLine):
        row = 4 + int(incomingLine[10])
        clearLineShort(row,130)
        screen.addstr(row,130,incomingLine)
    if ('IC 5 C' in incomingLine):
        row = 14 + int(incomingLine[10])
        clearLineShort(row,130)
        screen.addstr(row,130,incomingLine)
    if ('IC 6 C' in incomingLine):
        row = 24 + int(incomingLine[10])
        clearLineShort(row,130)
        screen.addstr(row,130,incomingLine)
    if ('IC 7 C' in incomingLine):
        row = 34 + int(incomingLine[10])
        clearLineShort(row,130)
        screen.addstr(row,130,incomingLine)
    if ('IC 0 T' in incomingLine):
        row = 4 + int(incomingLine[11])
        clearLineShort(row,155)
        screen.addstr(row,155,incomingLine)
    if ('IC 1 T' in incomingLine):
        row = 8 + int(incomingLine[11])
        clearLineShort(row,155)
        screen.addstr(row,155,incomingLine)
    if ('IC 2 T' in incomingLine):
        row = 12 + int(incomingLine[11])
        clearLineShort(row,155)
        screen.addstr(row,155,incomingLine)
    if ('IC 3 T' in incomingLine):
        row = 16 + int(incomingLine[11])
        clearLineShort(row,155)
        screen.addstr(row,155,incomingLine)
    if ('IC 4 T' in incomingLine):
        row = 4 + int(incomingLine[11])
        clearLineShort(row,180)
        screen.addstr(row,180,incomingLine)
    if ('IC 5 T' in incomingLine):
        row = 8 + int(incomingLine[11])
        clearLineShort(row,180)
        screen.addstr(row,180,incomingLine)
    if ('IC 6 T' in incomingLine):
        row = 12 + int(incomingLine[11])
        clearLineShort(row,180)
        screen.addstr(row,180,incomingLine)
    if ('IC 7 T' in incomingLine):
        row = 16 + int(incomingLine[11])
        clearLineShort(row,180)
        screen.addstr(row,180,incomingLine)
    screen.refresh()

def clearLine(y, x):
    blanks = '                                                  '
    screen.addstr(y,x,blanks)

def clearLineShort(y, x):
    blanks = '                      '
    screen.addstr(y,x,blanks)

def unpack(frame):
    #print("----------------")
    frame = ''.join(char for char in frame if char.isalnum())
    if (len(frame) != 32):
        # TODO throw an error up on screen
        #print("Malformed frame len " + str(len(frame)) + " encountered - skipping")
        return -1
    '''frameprint = ''
    odd = False
    for char in frame:
        frameprint += char
        if odd:
            frameprint += " "
        odd = not odd
    print("Encoded frame: " + frameprint.upper())'''
    try:
        decoded = cobs.decode(binascii.unhexlify(frame))
    except Exception as e:
        #print("Decode failed: " + str(e))
        return -1
    # Calculate checksum
    checksum = fletcher16(decoded[0:13])
    cs_calc = binascii.hexlify(chr(checksum >> 8)).upper() + " " + binascii.hexlify(chr(checksum & 0xFF)).upper()
    cs_rcvd = binascii.hexlify(decoded[14]).upper() + " " + binascii.hexlify(decoded[13]).upper()
    if cs_calc != cs_rcvd:
        #print("Decode failed: Checksum mismatch - calc: " + cs_calc + " - rcvd: " + cs_rcvd)
        return -1
    '''out = "Decoded frame: "
    for char in decoded:
        out += binascii.hexlify(char).upper() + " "
    print(out)'''
    return decoded

def decode(msg):
    ret = []
    id = ord(msg[0])
    #print("CAN ID:        0x" + binascii.hexlify(msg[0]).upper())
    size = ord(msg[4])
    #print("MSG LEN:       " + str(size))
    if (id == 0xA0):
        ret.append("MODULE A TEMP: " + str(b2i16(msg[5:7]) / 10.) + " C")
        ret.append("MODULE B TEMP: " + str(b2i16(msg[7:9]) / 10.) + " C")
        ret.append("MODULE C TEMP: " + str(b2i16(msg[9:11]) / 10.) + " C")
        ret.append("GATE DRIVER BOARD TEMP: " + str(b2i16(msg[11:13]) / 10.) + " C")
    if (id == 0xA2):
        ret.append("RTD 4 TEMP: " + str(b2i16(msg[5:7]) / 10.) + " C")
        ret.append("RTD 5 TEMP: " + str(b2i16(msg[7:9]) / 10.) + " C")
        ret.append("MOTOR TEMP: " + str(b2i16(msg[9:11]) / 10.) + " C")
        ret.append("TORQUE SHUDDER: " + str(b2i16(msg[11:13]) / 10.) + " Nm")
    if (id == 0xA5):
        ret.append("MOTOR ANGLE: " + str(b2i16(msg[5:7]) / 10.))
        ret.append("MOTOR SPEED: " + str(b2i16(msg[7:9])) + " RPM")
        ret.append("ELEC OUTPUT FREQ: " + str(b2i16(msg[9:11]) / 10.))
        ret.append("DELTA RESOLVER FILT: " + str(b2i16(msg[11:13])))
    if (id == 0xA6):
        ret.append("PHASE A CURRENT: " + str(b2i16(msg[5:7]) / 10.) + " A")
        ret.append("PHASE B CURRENT: " + str(b2i16(msg[7:9]) / 10.) + " A")
        ret.append("PHASE C CURRENT: " + str(b2i16(msg[9:11]) / 10.) + " A")
        ret.append("DC BUS CURRENT: " + str(b2i16(msg[11:13]) / 10.) + " A")
    if (id == 0xA7):
        ret.append("DC BUS VOLTAGE: " + str(b2i16(msg[5:7]) / 10.) + " V")
        ret.append("OUTPUT VOLTAGE: " + str(b2i16(msg[7:9]) / 10.) + " V")
        ret.append("PHASE AB VOLTAGE: " + str(b2i16(msg[9:11]) / 10.) + " V")
        ret.append("PHASE BC VOLTAGE: " + str(b2i16(msg[11:13]) / 10.) + " V")
    if (id == 0xAA):
        ret.append("VSM STATE: " + str(b2ui16(msg[5:7])))
        ret.append("INVERTER STATE: " + str(ord(msg[7])))
        ret.append("INVERTER RUN MODE: " + str(ord(msg[9]) & 0x1))
        ret.append("INVERTER ACTIVE DISCHARGE STATE: " + str((ord(msg[9]) & 0xE0) >> 5))
        ret.append("INVERTER COMMAND MODE: " + str(ord(msg[10])))
        ret.append("INVERTER ENABLE: " + str(ord(msg[11]) & 0x1))
        ret.append("INVERTER LOCKOUT: " + str((ord(msg[11]) & 0x80) >> 7))
        ret.append("DIRECTION COMMAND: " + str(ord(msg[12])))
    if (id == 0xAB):
        ret.append("POST FAULT LO: 0x" + binascii.hexlify(msg[6]).upper() + binascii.hexlify(msg[5]).upper())
        ret.append("POST FAULT HI: 0x" + binascii.hexlify(msg[8]).upper() + binascii.hexlify(msg[7]).upper())
        ret.append("RUN FAULT LO: 0x" + binascii.hexlify(msg[10]).upper() + binascii.hexlify(msg[9]).upper())
        ret.append("RUN FAULT HI: 0x" + binascii.hexlify(msg[12]).upper() + binascii.hexlify(msg[11]).upper())
    if (id == 0xAC):
        ret.append("COMMANDED TORQUE: " + str(b2i16(msg[5:7]) / 10.) + " Nm")
        ret.append("TORQUE FEEDBACK: " + str(b2i16(msg[7:9]) / 10.) + " Nm")
        ret.append("RMS UPTIME: " + str(int(b2ui32(msg[9:13]) * .003)) + " s")
    if (id == 0xC0):
        ret.append("FCU REQUESTED TORQUE: " + str(b2i16(msg[5:7]) / 10.) + " N")
        #ret.append("FCU REQUESTED INVERTER ENABLE: " + str(ord(msg[10]) & 0x1))
    if (id == 0xD0):
        ret.append("RCU STATE: " + str(ord(msg[5])))
        ret.append("RCU FLAGS: 0x" + binascii.hexlify(msg[6]).upper())
        ret.append("GLV BATT VOLTAGE: " + str(b2ui16(msg[7:9]) / 100.) + " V")
        ret.append("RCU BMS FAULT: " + str(not ord(msg[6]) & 0x1))
        ret.append("RCU IMD FAULT: " + str(not (ord(msg[6]) & 0x2) >> 1))
    if (id == 0xD2):
        ret.append("FCU STATE: " + str(ord(msg[5])))
        ret.append("FCU FLAGS: 0x" + binascii.hexlify(msg[6]).upper())
        ret.append("FCU START BUTTON ID: " + str(ord(msg[7])))
        ret.append("FCU BRAKE ACT: " + str((ord(msg[6]) & 0x8) >> 3))
        ret.append("FCU IMPLAUS ACCEL: " + str(ord(msg[6]) & 0x1))
        ret.append("FCU IMPLAUS BRAKE: " + str((ord(msg[6]) & 0x4) >> 2))
    if (id == 0xD3):
        ret.append("FCU PEDAL ACCEL 1: " + str(b2ui16(msg[5:7])))
        ret.append("FCU PEDAL ACCEL 2: " + str(b2ui16(msg[7:9])))
        ret.append("FCU PEDAL BRAKE: " + str(b2ui16(msg[9:11])))
    if (id == 0xD7):
        ret.append("BMS VOLTAGE AVERAGE: " + str(b2ui16(msg[5:7]) / 10e3) + " V")
        ret.append("BMS VOLTAGE LOW: " + str(b2ui16(msg[7:9]) / 10e3) + " V")
        ret.append("BMS VOLTAGE HIGH: " + str(b2ui16(msg[9:11]) / 10e3) + " V")
        ret.append("BMS VOLTAGE TOTAL: " + str(b2ui16(msg[11:13]) / 100.) + " V")
    if (id == 0xD8):
        ic = ord(msg[5]) & 0xF
        group = (ord(msg[5]) & 0xF0) >> 4
        ret.append("IC " + str(ic) + " CELL " + str(group * 3) + ": " + '%.4f' % (b2ui16(msg[6:8]) / 10e3) + " V")
        ret.append("IC " + str(ic) + " CELL " + str(group * 3 + 1) + ": " + '%.4f' % (b2ui16(msg[8:10]) / 10e3) + " V")
        ret.append("IC " + str(ic) + " CELL " + str(group * 3 + 2) + ": " + '%.4f' % (b2ui16(msg[10:12]) / 10e3) + " V")
    if (id == 0xD9):
        ret.append("BMS AVERAGE TEMPERATURE: " + str(b2i16(msg[5:7]) / 100.) + " C")
        ret.append("BMS LOW TEMPERATURE: " + str(b2i16(msg[7:9]) / 100.) + " C")
        ret.append("BMS HIGH TEMPERATURE: " + str(b2i16(msg[9:11]) / 100.) + " C")
    if (id == 0xDA):
        ic = ord(msg[5])
        ret.append("IC " + str(ic) + " THERM 0: " + '%.2f' % (b2ui16(msg[6:8]) / 100.) + " C")
        ret.append("IC " + str(ic) + " THERM 1: " + '%.2f' % (b2ui16(msg[8:10]) / 100.) + " C")
        ret.append("IC " + str(ic) + " THERM 2: " + '%.2f' % (b2ui16(msg[10:12]) / 100.) + " C")
    if (id == 0xDB):
        ret.append("BMS STATE: " + str(ord(msg[5])))
        ret.append("BMS ERROR FLAGS: 0x" + binascii.hexlify(msg[7]).upper() + binascii.hexlify(msg[6]).upper())
        ret.append("BMS CURRENT: " + str(b2i16(msg[8:10]) / 100.) + " A")
    return ret

def b2i8(data):
    return struct.unpack("<1b", chr(ord(data[0])))[0]

def b2i16(data):
    return struct.unpack("<1h", chr(ord(data[0])) + chr(ord(data[1])))[0]

def b2ui16(data):
    return struct.unpack("<1H", chr(ord(data[0])) + chr(ord(data[1])))[0]

def b2ui32(data):
    return struct.unpack("<1I", chr(ord(data[0])) + chr(ord(data[1])) + chr(ord(data[2])) + chr(ord(data[3])))[0]

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
