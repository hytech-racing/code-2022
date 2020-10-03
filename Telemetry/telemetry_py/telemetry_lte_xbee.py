import curses
import sys
import datetime
import binascii
from cobs import cobs
import codecs
import struct
import paho.mqtt.client as mqtt

class TelemetryClient:
    def __init__(self):
        self.screen = None
        self.ecu_version = 0
        self.countGoodFrames = 0
        self.countBadFrames = 0

        # Set up filenames for logging
        timestamp = str(datetime.datetime.now())
        self.filename = timestamp + ".txt"
        self.filenameRaw = "raw " + timestamp + ".csv"


    def mqtt_connect(self, client, userdata, flags, rc):
        client.subscribe("hytech_car/telemetry")
        self.screen.addstr(0,47,' - CONNECTED')
        client.publish("hytech_car/telemetry", "Python client connected")

    def mqtt_message(self, client, userdata, msg):
        self.screen.addstr(0,59,' - RECEIVED')

        # TODO check format of incoming message for errors
        timestamp = msg.payload[0:msg.payload.find(b',')]
        frame = msg.payload[msg.payload.find(b',') + 1:-1]
        frame = binascii.hexlify(frame)
        data = unpack("".join(chr(c) for c in frame))

        if data != -1:
            # with open(self.filenameRaw, "a") as f:
            #     size = ord(data[4])
            #     raw = binascii.hexlify(data[0]).upper() + "," + binascii.hexlify(data[5:5 + size]).upper()
            #     f.write(str(datetime.datetime.now()) + ',' + raw + "\n")
            self.countGoodFrames += 1
            lines = decode(data)
            with open(self.filename, "a") as f:
                # TODO update to use timestamp sent over LTE instead of local timestamp
                timestamp = str(datetime.datetime.now())
                for line in lines:
                    self.updateScreen(line)
                    f.write(timestamp + ' ' + line + '\n')
        else:
            self.countBadFrames += 1

    def setupScreen(self, initScreen):
        self.screen = initScreen

        if sys.argv[1] == 'fcu':
            self.ecu_version = 0
        else:
            self.ecu_version = 1

        self.screen.border('#', '#', '#', '#', 0, 0, 0, 0)
        self.screen.addstr(0,5,'HYTECH RACING 2019 VEHICLE SERIAL DEBUGGER')
        self.screen.addstr(3,5,'RMS INVERTER')
        self.screen.addstr(4,5,'MODULE A TEMP: ')
        self.screen.addstr(5,5,'MODULE B TEMP: ')
        self.screen.addstr(6,5,'MODULE C TEMP: ')
        self.screen.addstr(7,5,'GATE DRIVER BOARD TEMP: ')
        self.screen.addstr(8,5,'RTD 4 TEMP: ')
        self.screen.addstr(9,5,'RTD 5 TEMP: ')
        self.screen.addstr(10,5,'MOTOR TEMP: ')
        self.screen.addstr(11,5,'TORQUE SHUDDER: ')
        self.screen.addstr(12,5,'MOTOR ANGLE: ')
        self.screen.addstr(13,5,'MOTOR SPEED: ')
        self.screen.addstr(14,5,'ELEC OUTPUT FREQ: ')
        self.screen.addstr(15,5,'DELTA RESOLVER FILT: ')
        self.screen.addstr(16,5,'PHASE A CURRENT: ')
        self.screen.addstr(17,5,'PHASE B CURRENT: ')
        self.screen.addstr(18,5,'PHASE C CURRENT: ')
        self.screen.addstr(19,5,'DC BUS CURRENT: ')
        self.screen.addstr(20,5,'DC BUS VOLTAGE: ')
        self.screen.addstr(21,5,'OUTPUT VOLTAGE: ')
        self.screen.addstr(22,5,'PHASE AB VOLTAGE: ')
        self.screen.addstr(23,5,'PHASE BC VOLTAGE: ')
        self.screen.addstr(24,5,'VSM STATE: ')
        self.screen.addstr(25,5,'INVERTER STATE: ')
        self.screen.addstr(26,5,'INVERTER RUN MODE: ')
        self.screen.addstr(27,5,'INVERTER ACTIVE DISCHARGE STATE: ')
        self.screen.addstr(28,5,'INVERTER COMMAND MODE: ')
        self.screen.addstr(29,5,'INVERTER ENABLE: ')
        self.screen.addstr(30,5,'INVERTER LOCKOUT: ')
        self.screen.addstr(31,5,'DIRECTION COMMAND: ')
        self.screen.addstr(32,5,'POST FAULT LO: ')
        self.screen.addstr(33,5,'POST FAULT HI: ')
        self.screen.addstr(34,5,'RUN FAULT LO: ')
        self.screen.addstr(35,5,'RUN FAULT HI: ')
        self.screen.addstr(36,5,'COMMANDED TORQUE: ')
        self.screen.addstr(37,5,'TORQUE FEEDBACK: ')
        self.screen.addstr(38,5,'RMS UPTIME: ')

        self.screen.addstr(40,5,'GLV CURRENT READINGS')
        self.screen.addstr(41,5,'ECU CURRENT: ')
        self.screen.addstr(42,5,'COOLING CURRENT: ')

        self.screen.addstr(3,55,'BATTERY MANAGEMENT SYSTEM')
        self.screen.addstr(4,55,'BMS AVERAGE TEMPERATURE: ')
        self.screen.addstr(5,55,'BMS LOW TEMPERATURE: ')
        self.screen.addstr(6,55,'BMS HIGH TEMPERATURE: ')
        self.screen.addstr(7,55,'BMS STATE: ')
        self.screen.addstr(8,55,'BMS ERROR FLAGS: ')
        self.screen.addstr(9,55,'BMS CURRENT: ')
        self.screen.addstr(10,55,'BMS VOLTAGE AVERAGE: ')
        self.screen.addstr(11,55,'BMS VOLTAGE LOW: ')
        self.screen.addstr(12,55,'BMS VOLTAGE HIGH: ')
        self.screen.addstr(13,55,'BMS VOLTAGE TOTAL: ')
        self.screen.addstr(14,55,'BMS TOTAL CHARGE: ')
        self.screen.addstr(15,55,'BMS TOTAL DISCHARGE: ')

        if self.ecu_version == 0:
            self.screen.addstr(17,55,'FRONT CONTROL UNIT')
            self.screen.addstr(18,55,'FCU UPTIME: ')
            self.screen.addstr(19,55,'FCU STATE: ')
            self.screen.addstr(20,55,'START BUTTON ID: ')
            self.screen.addstr(21,55,'FCU RAW TORQUE: ')
            self.screen.addstr(22,55,'REQUESTED TORQUE: ')

            self.screen.addstr(23,55,'FCU PEDAL ACCEL 1: ')
            self.screen.addstr(24,55,'FCU PEDAL ACCEL 2: ')
            self.screen.addstr(25,55,'FCU PEDAL BRAKE: ')
            self.screen.addstr(26,55,'FCU BRAKE ACT: ')
            self.screen.addstr(27,55,'FCU IMPLAUS ACCEL: ')
            self.screen.addstr(28,55,'FCU IMPLAUS BRAKE: ')

            # RCU messages are only needed if FCU is used
            self.screen.addstr(30,55,'REAR CONTROL UNIT')
            self.screen.addstr(31,55,'RCU UPTIME: ')
            self.screen.addstr(32,55,'RCU STATE: ')
            self.screen.addstr(33,55,'RCU FLAGS: ')
            self.screen.addstr(34,55,'GLV BATT VOLTAGE: ')
            self.screen.addstr(35,55,'RCU BMS FAULT: ')
            self.screen.addstr(36,55,'RCU IMD FAULT: ')
        else:
            self.screen.addstr(17,55,'MAIN CONTROL UNIT')
            self.screen.addstr(18,55,'MCU STATE: ')
            self.screen.addstr(19,55,'MCU BMS FAULT: ')
            self.screen.addstr(20,55,'MCU IMD FAULT: ')
            self.screen.addstr(21,55,'MCU INVERTER POWER: ')
            self.screen.addstr(22,55,'MCU SHUTDOWN ABOVE THRESH: ')
            self.screen.addstr(23,55,'MCU TEMPERATURE: ')
            self.screen.addstr(24,55,'MCU GLV VOLTAGE: ')

            self.screen.addstr(25,55,'MCU PEDAL ACCEL 1: ')
            self.screen.addstr(26,55,'MCU PEDAL ACCEL 2: ')
            self.screen.addstr(27,55,'MCU PEDAL BRAKE: ')
            self.screen.addstr(28,55,'MCU BRAKE ACT: ')
            self.screen.addstr(29,55,'MCU IMPLAUS ACCEL: ')
            self.screen.addstr(30,55,'MCU IMPLAUS BRAKE: ')
            self.screen.addstr(31,55,'MCU TORQUE MAP MODE: ')
            self.screen.addstr(32,55,'REQUESTED TORQUE: ')

        self.screen.addstr(38,55,'TCU WHEEL RPM REAR LEFT: ')
        self.screen.addstr(39,55,'TCU WHEEL RPM REAR RIGHT: ')
        self.screen.addstr(40,55,'TCU WHEEL RPM FRONT LEFT: ')
        self.screen.addstr(41,55,'TCU WHEEL RPM FRONT RIGHT: ')

        self.screen.addstr(46,55,'TCU DISTANCE TRAVELED: ')
        self.screen.addstr(3,105,'BATTERY MANAGEMENT SYSTEM DETAILED VOLTAGES')
        self.screen.addstr(4,105,'IC 0 CELL 0: ')
        self.screen.addstr(5,105,'IC 0 CELL 1: ')
        self.screen.addstr(6,105,'IC 0 CELL 2: ')
        self.screen.addstr(7,105,'IC 0 CELL 3: ')
        self.screen.addstr(8,105,'IC 0 CELL 4: ')
        self.screen.addstr(9,105,'IC 0 CELL 5: ')
        self.screen.addstr(10,105,'IC 0 CELL 6: ')
        self.screen.addstr(11,105,'IC 0 CELL 7: ')
        self.screen.addstr(12,105,'IC 0 CELL 8: ')

        self.screen.addstr(14,105,'IC 1 CELL 0: ')
        self.screen.addstr(15,105,'IC 1 CELL 1: ')
        self.screen.addstr(16,105,'IC 1 CELL 2: ')
        self.screen.addstr(17,105,'IC 1 CELL 3: ')
        self.screen.addstr(18,105,'IC 1 CELL 4: ')
        self.screen.addstr(19,105,'IC 1 CELL 5: ')
        self.screen.addstr(20,105,'IC 1 CELL 6: ')
        self.screen.addstr(21,105,'IC 1 CELL 7: ')
        self.screen.addstr(22,105,'IC 1 CELL 8: ')

        self.screen.addstr(24,105,'IC 2 CELL 0: ')
        self.screen.addstr(25,105,'IC 2 CELL 1: ')
        self.screen.addstr(26,105,'IC 2 CELL 2: ')
        self.screen.addstr(27,105,'IC 2 CELL 3: ')
        self.screen.addstr(28,105,'IC 2 CELL 4: ')
        self.screen.addstr(29,105,'IC 2 CELL 5: ')
        self.screen.addstr(30,105,'IC 2 CELL 6: ')
        self.screen.addstr(31,105,'IC 2 CELL 7: ')
        self.screen.addstr(32,105,'IC 2 CELL 8: ')

        self.screen.addstr(34,105,'IC 3 CELL 0: ')
        self.screen.addstr(35,105,'IC 3 CELL 1: ')
        self.screen.addstr(36,105,'IC 3 CELL 2: ')
        self.screen.addstr(37,105,'IC 3 CELL 3: ')
        self.screen.addstr(38,105,'IC 3 CELL 4: ')
        self.screen.addstr(39,105,'IC 3 CELL 5: ')
        self.screen.addstr(40,105,'IC 3 CELL 6: ')
        self.screen.addstr(41,105,'IC 3 CELL 7: ')
        self.screen.addstr(42,105,'IC 3 CELL 8: ')

        self.screen.addstr(4,130,'IC 4 CELL 0: ')
        self.screen.addstr(5,130,'IC 4 CELL 1: ')
        self.screen.addstr(6,130,'IC 4 CELL 2: ')
        self.screen.addstr(7,130,'IC 4 CELL 3: ')
        self.screen.addstr(8,130,'IC 4 CELL 4: ')
        self.screen.addstr(9,130,'IC 4 CELL 5: ')
        self.screen.addstr(10,130,'IC 4 CELL 6: ')
        self.screen.addstr(11,130,'IC 4 CELL 7: ')
        self.screen.addstr(12,130,'IC 4 CELL 8: ')

        self.screen.addstr(14,130,'IC 5 CELL 0: ')
        self.screen.addstr(15,130,'IC 5 CELL 1: ')
        self.screen.addstr(16,130,'IC 5 CELL 2: ')
        self.screen.addstr(17,130,'IC 5 CELL 3: ')
        self.screen.addstr(18,130,'IC 5 CELL 4: ')
        self.screen.addstr(19,130,'IC 5 CELL 5: ')
        self.screen.addstr(20,130,'IC 5 CELL 6: ')
        self.screen.addstr(21,130,'IC 5 CELL 7: ')
        self.screen.addstr(22,130,'IC 5 CELL 8: ')

        self.screen.addstr(24,130,'IC 6 CELL 0: ')
        self.screen.addstr(25,130,'IC 6 CELL 1: ')
        self.screen.addstr(26,130,'IC 6 CELL 2: ')
        self.screen.addstr(27,130,'IC 6 CELL 3: ')
        self.screen.addstr(28,130,'IC 6 CELL 4: ')
        self.screen.addstr(29,130,'IC 6 CELL 5: ')
        self.screen.addstr(30,130,'IC 6 CELL 6: ')
        self.screen.addstr(31,130,'IC 6 CELL 7: ')
        self.screen.addstr(32,130,'IC 6 CELL 8: ')

        self.screen.addstr(34,130,'IC 7 CELL 0: ')
        self.screen.addstr(35,130,'IC 7 CELL 1: ')
        self.screen.addstr(36,130,'IC 7 CELL 2: ')
        self.screen.addstr(37,130,'IC 7 CELL 3: ')
        self.screen.addstr(38,130,'IC 7 CELL 4: ')
        self.screen.addstr(39,130,'IC 7 CELL 5: ')
        self.screen.addstr(40,130,'IC 7 CELL 6: ')
        self.screen.addstr(41,130,'IC 7 CELL 7: ')
        self.screen.addstr(42,130,'IC 7 CELL 8: ')

        self.screen.addstr(3,155,'BATTERY MANAGEMENT SYSTEM DETAILED TEMPERATURES')
        self.screen.addstr(4,155,'IC 0 THERM 0: ')
        self.screen.addstr(5,155,'IC 0 THERM 1: ')
        self.screen.addstr(6,155,'IC 0 THERM 2: ')

        self.screen.addstr(8,155,'IC 1 THERM 0: ')
        self.screen.addstr(9,155,'IC 1 THERM 1: ')
        self.screen.addstr(10,155,'IC 1 THERM 2: ')

        self.screen.addstr(12,155,'IC 2 THERM 0: ')
        self.screen.addstr(13,155,'IC 2 THERM 1: ')
        self.screen.addstr(14,155,'IC 2 THERM 2: ')

        self.screen.addstr(16,155,'IC 3 THERM 0: ')
        self.screen.addstr(17,155,'IC 3 THERM 1: ')
        self.screen.addstr(18,155,'IC 3 THERM 2: ')

        self.screen.addstr(4,180,'IC 4 THERM 0: ')
        self.screen.addstr(5,180,'IC 4 THERM 1: ')
        self.screen.addstr(6,180,'IC 4 THERM 2: ')

        self.screen.addstr(8,180,'IC 5 THERM 0: ')
        self.screen.addstr(9,180,'IC 5 THERM 1: ')
        self.screen.addstr(10,180,'IC 5 THERM 2: ')

        self.screen.addstr(12,180,'IC 6 THERM 0: ')
        self.screen.addstr(13,180,'IC 6 THERM 1: ')
        self.screen.addstr(14,180,'IC 6 THERM 2: ')

        self.screen.addstr(16,180,'IC 7 THERM 0: ')
        self.screen.addstr(17,180,'IC 7 THERM 1: ')
        self.screen.addstr(18,180,'IC 7 THERM 2: ')

        self.screen.addstr(20,155,'BATTERY MANAGEMENT SYSTEM BALANCING STATUS')
        self.screen.addstr(21,160,'C0')
        self.screen.addstr(21,165,'C1')
        self.screen.addstr(21,170,'C2')
        self.screen.addstr(21,175,'C3')
        self.screen.addstr(21,180,'C4')
        self.screen.addstr(21,185,'C5')
        self.screen.addstr(21,190,'C6')
        self.screen.addstr(21,195,'C7')
        self.screen.addstr(21,200,'C8')
        self.screen.addstr(22,155,'IC0')
        self.screen.addstr(23,155,'IC1')
        self.screen.addstr(24,155,'IC2')
        self.screen.addstr(25,155,'IC3')
        self.screen.addstr(26,155,'IC4')
        self.screen.addstr(27,155,'IC5')
        self.screen.addstr(28,155,'IC6')
        self.screen.addstr(29,155,'IC7')

        self.live()
        curses.endwin()

    def live(self):
        # Set up mqtt connection
        client = mqtt.Client()
        client.connect("ec2-3-134-2-166.us-east-2.compute.amazonaws.com", 1883, 60)
        client.on_connect = self.mqtt_connect
        client.on_message = self.mqtt_message
        client.loop_start()

        self.screen.nodelay(True)

        # Write CSV Header
        with open(self.filenameRaw, "a") as f:
            f.write("timestamp,CAN ID,msg\n")

        # Wait for q to quit
        char = self.screen.getch()
        while char != ord('q') and char != ord('Q'):
            char = self.screen.getch()

        # Time to quit, disconnect MQTT
        client.loop_stop()
        client.disconnect() # TODO unsure if this should be called

        # Write out statistics
        with open(self.filename, "a") as f:
            f.write("Processed " + str(self.countGoodFrames) + " good frames - ignored " + str(self.countBadFrames) + " bad frames")

    def updateScreen(self, incomingLine):
        if ('MODULE A TEMP' in incomingLine):
            self.clearLine(4,5)
            self.screen.addstr(4,5,incomingLine)
        if ('MODULE B TEMP' in incomingLine):
            self.clearLine(5,5)
            self.screen.addstr(5,5,incomingLine)
        if ('MODULE C TEMP' in incomingLine):
            self.clearLine(6,5)
            self.screen.addstr(6,5,incomingLine)
        if ('GATE DRIVER BOARD TEMP' in incomingLine):
            self.clearLine(7,5)
            self.screen.addstr(7,5,incomingLine)
        if ('RTD 4 TEMP' in incomingLine):
            self.clearLine(8,5)
            self.screen.addstr(8,5,incomingLine)
        if ('RTD 5 TEMP' in incomingLine):
            self.clearLine(9,5)
            self.screen.addstr(9,5,incomingLine)
        if ('MOTOR TEMP' in incomingLine):
            self.clearLine(10,5)
            self.screen.addstr(10,5,incomingLine)
        if ('TORQUE SHUDDER' in incomingLine):
            self.clearLine(11,5)
            self.screen.addstr(11,5,incomingLine)
        if ('MOTOR ANGLE' in incomingLine):
            self.clearLine(12,5)
            self.screen.addstr(12,5,incomingLine)
        if ('MOTOR SPEED' in incomingLine):
            self.clearLine(13,5)
            self.screen.addstr(13,5,incomingLine)
        if ('ELEC OUTPUT FREQ' in incomingLine):
            self.clearLine(14,5)
            self.screen.addstr(14,5,incomingLine)
        if ('DELTA RESOLVER FILT' in incomingLine):
            self.clearLine(15,5)
            self.screen.addstr(15,5,incomingLine)
        if ('PHASE A CURRENT' in incomingLine):
            self.clearLine(16,5)
            self.screen.addstr(16,5,incomingLine)
        if ('PHASE B CURRENT' in incomingLine):
            self.clearLine(17,5)
            self.screen.addstr(17,5,incomingLine)
        if ('PHASE C CURRENT' in incomingLine):
            self.clearLine(18,5)
            self.screen.addstr(18,5,incomingLine)
        if ('DC BUS CURRENT' in incomingLine):
            self.clearLine(19,5)
            self.screen.addstr(19,5,incomingLine)
        if ('DC BUS VOLTAGE' in incomingLine):
            self.clearLine(20,5)
            self.screen.addstr(20,5,incomingLine)
        if ('OUTPUT VOLTAGE' in incomingLine):
            self.clearLine(21,5)
            self.screen.addstr(21,5,incomingLine)
        if ('PHASE AB VOLTAGE' in incomingLine):
            self.clearLine(22,5)
            self.screen.addstr(22,5,incomingLine)
        if ('PHASE BC VOLTAGE' in incomingLine):
            self.clearLine(23,5)
            self.screen.addstr(23,5,incomingLine)
        if ('VSM STATE' in incomingLine):
            self.clearLine(24,5)
            self.screen.addstr(24,5,incomingLine)
        if ('INVERTER STATE' in incomingLine):
            self.clearLine(25,5)
            self.screen.addstr(25,5,incomingLine)
        if ('INVERTER RUN MODE' in incomingLine):
            self.clearLine(26,5)
            self.screen.addstr(26,5,incomingLine)
        if ('INVERTER ACTIVE DISCHARGE STATE' in incomingLine):
            self.clearLine(27,5)
            self.screen.addstr(27,5,incomingLine)
        if ('INVERTER COMMAND MODE' in incomingLine):
            self.clearLine(28,5)
            self.screen.addstr(28,5,incomingLine)
        if ('INVERTER ENABLE' in incomingLine):
            self.clearLine(29,5)
            self.screen.addstr(29,5,incomingLine)
        if ('INVERTER LOCKOUT' in incomingLine):
            self.clearLine(30,5)
            self.screen.addstr(30,5,incomingLine)
        if ('DIRECTION COMMAND' in incomingLine):
            self.clearLine(31,5)
            self.screen.addstr(31,5,incomingLine)
        if ('POST FAULT LO' in incomingLine):
            self.clearLine(32,5)
            self.screen.addstr(32,5,incomingLine)
        if ('POST FAULT HI' in incomingLine):
            self.clearLine(33,5)
            self.screen.addstr(33,5,incomingLine)
        if ('RUN FAULT LO' in incomingLine):
            self.clearLine(34,5)
            self.screen.addstr(34,5,incomingLine)
        if ('RUN FAULT HI' in incomingLine):
            self.clearLine(35,5)
            self.screen.addstr(35,5,incomingLine)
        if ('COMMANDED TORQUE:' in incomingLine):
            self.clearLine(36,5)
            self.screen.addstr(36,5,incomingLine)
        if ('TORQUE FEEDBACK' in incomingLine):
            self.clearLine(37,5)
            self.screen.addstr(37,5,incomingLine)
        if ('RMS UPTIME' in incomingLine):
            self.clearLine(38,5)
            self.screen.addstr(38,5,incomingLine)
        if ('ECU CURRENT' in incomingLine):
            self.clearLine(41,5)
            self.screen.addstr(41,5,incomingLine)
        if ('COOLING CURRENT' in incomingLine):
            self.clearLine(42,5)
            self.screen.addstr(42,5,incomingLine)
        if ('BMS AVERAGE TEMPERATURE' in incomingLine):
            self.clearLine(4,55)
            self.screen.addstr(4,55,incomingLine)
        if ('BMS LOW TEMPERATURE' in incomingLine):
            self.clearLine(5,55)
            self.screen.addstr(5,55,incomingLine)
        if ('BMS HIGH TEMPERATURE' in incomingLine):
            self.clearLine(6,55)
            self.screen.addstr(6,55,incomingLine)
        if ('BMS STATE' in incomingLine):
            self.clearLine(7,55)
            self.screen.addstr(7,55,incomingLine)
        if ('BMS ERROR FLAGS' in incomingLine):
            self.clearLine(8,55)
            self.screen.addstr(8,55,incomingLine)
        if ('BMS CURRENT' in incomingLine):
            self.clearLine(9,55)
            self.screen.addstr(9,55,incomingLine)
        if ('BMS VOLTAGE AVERAGE' in incomingLine):
            self.clearLine(10,55)
            self.screen.addstr(10,55,incomingLine)
        if ('BMS VOLTAGE LOW' in incomingLine):
            self.clearLine(11,55)
            self.screen.addstr(11,55,incomingLine)
        if ('BMS VOLTAGE HIGH' in incomingLine):
            self.clearLine(12,55)
            self.screen.addstr(12,55,incomingLine)
        if ('BMS VOLTAGE TOTAL' in incomingLine):
            self.clearLine(13,55)
            self.screen.addstr(13,55,incomingLine)
        if ('BMS TOTAL CHARGE' in incomingLine):
            self.clearLine(14,55)
            self.screen.addstr(14,55,incomingLine)
        if ('BMS TOTAL DISCHARGE' in incomingLine):
            self.clearLine(15,55)
            self.screen.addstr(15,55,incomingLine)
        if ('FCU UPTIME' in incomingLine or 'MCU STATE' in incomingLine):
            self.clearLine(18,55)
            self.screen.addstr(18,55,incomingLine)
        if ('FCU STATE' in incomingLine or 'MCU BMS FAULT' in incomingLine):
            self.clearLine(19,55)
            self.screen.addstr(19,55,incomingLine)
        if ('FCU START BUTTON ID' in incomingLine or 'MCU IMD FAULT' in incomingLine):
            self.clearLine(20,55)
            self.screen.addstr(20,55,incomingLine)
        if ('FCU RAW TORQUE' in incomingLine or 'MCU INVERTER POWER' in incomingLine):
            self.clearLine(21,55)
            self.screen.addstr(21,55,incomingLine)
        if ('REQUESTED TORQUE' in incomingLine):
            if self.ecu_version == 0:
                self.clearLine(22,55)
                self.screen.addstr(22,55,incomingLine)
            else:
                self.clearLine(32,55)
                self.screen.addstr(32,55,incomingLine)
        if ('MCU SHUTDOWN ABOVE THRESH' in incomingLine):
            self.clearLine(22,55)
            self.screen.addstr(22,55,incomingLine)
        if ('FCU PEDAL ACCEL 1' in incomingLine or 'MCU TEMPERATURE' in incomingLine):
            self.clearLine(23,55)
            self.screen.addstr(23,55,incomingLine)
        if ('FCU PEDAL ACCEL 2' in incomingLine or 'MCU GLV VOLTAGE' in incomingLine):
            self.clearLine(24,55)
            self.screen.addstr(24,55,incomingLine)
        if ('FCU PEDAL BRAKE' in incomingLine or 'MCU PEDAL ACCEL 1' in incomingLine):
            self.clearLine(25,55)
            self.screen.addstr(25,55,incomingLine)
        if ('FCU BRAKE ACT' in incomingLine or 'MCU PEDAL ACCEL 2' in incomingLine):
            self.clearLine(26,55)
            self.screen.addstr(26,55,incomingLine)
        if ('FCU IMPLAUS ACCEL' in incomingLine or 'MCU PEDAL BRAKE' in incomingLine):
            self.clearLine(27,55)
            self.screen.addstr(27,55,incomingLine)
        if ('FCU IMPLAUS BRAKE' in incomingLine or 'MCU BRAKE ACT' in incomingLine):
            self.clearLine(28,55)
            self.screen.addstr(28,55,incomingLine)
        if ('MCU IMPLAUS ACCEL' in incomingLine):
            self.clearLine(29, 55)
            self.screen.addstr(29, 55, incomingLine)
        if ('MCU IMPLAUS BRAKE' in incomingLine):
            self.clearLine(30, 55)
            self.screen.addstr(30, 55, incomingLine)
        if ('RCU UPTIME' in incomingLine or 'MCU TORQUE MAP MODE' in incomingLine):
            self.clearLine(31,55)
            self.screen.addstr(31,55,incomingLine)
        if ('RCU STATE' in incomingLine):
            self.clearLine(32,55)
            self.screen.addstr(32,55,incomingLine)
        if ('RCU FLAGS' in incomingLine):
            self.clearLine(33,55)
            self.screen.addstr(33,55,incomingLine)
        if ('GLV BATT VOLTAGE' in incomingLine):
            self.clearLine(34,55)
            self.screen.addstr(34,55,incomingLine)
        if ('RCU BMS FAULT' in incomingLine):
            self.clearLine(35,55)
            self.screen.addstr(35,55,incomingLine)
        if ('RCU IMD FAULT' in incomingLine):
            self.clearLine(36,55)
            self.screen.addstr(36,55,incomingLine)
        if ('TCU WHEEL RPM REAR LEFT' in incomingLine):
            self.clearLine(38,55)
            self.screen.addstr(38,55,incomingLine)
        if ('TCU WHEEL RPM REAR RIGHT' in incomingLine):
            self.clearLine(39,55)
            self.screen.addstr(39,55,incomingLine)
        if ('TCU WHEEL RPM FRONT LEFT' in incomingLine):
            self.clearLine(40,55)
            self.screen.addstr(40,55,incomingLine)
        if ('TCU WHEEL RPM FRONT RIGHT' in incomingLine):
            self.clearLine(41,55)
            self.screen.addstr(41,55,incomingLine)
        if ('TCU DISTANCE TRAVELED' in incomingLine):
            self.clearLine(46,55)
            self.screen.addstr(46,55,incomingLine)
        if ('IC 0 C' in incomingLine):
            row = 4 + int(incomingLine[10])
            self.clearLineShort(row,105)
            self.screen.addstr(row,105,incomingLine)
        if ('IC 1 C' in incomingLine):
            row = 14 + int(incomingLine[10])
            self.clearLineShort(row,105)
            self.screen.addstr(row,105,incomingLine)
        if ('IC 2 C' in incomingLine):
            row = 24 + int(incomingLine[10])
            self.clearLineShort(row,105)
            self.screen.addstr(row,105,incomingLine)
        if ('IC 3 C' in incomingLine):
            row = 34 + int(incomingLine[10])
            self.clearLineShort(row,105)
            self.screen.addstr(row,105,incomingLine)
        if ('IC 4 C' in incomingLine):
            row = 4 + int(incomingLine[10])
            self.clearLineShort(row,130)
            self.screen.addstr(row,130,incomingLine)
        if ('IC 5 C' in incomingLine):
            row = 14 + int(incomingLine[10])
            self.clearLineShort(row,130)
            self.screen.addstr(row,130,incomingLine)
        if ('IC 6 C' in incomingLine):
            row = 24 + int(incomingLine[10])
            self.clearLineShort(row,130)
            self.screen.addstr(row,130,incomingLine)
        if ('IC 7 C' in incomingLine):
            row = 34 + int(incomingLine[10])
            self.clearLineShort(row,130)
            self.screen.addstr(row,130,incomingLine)
        if ('IC 0 T' in incomingLine):
            row = 4 + int(incomingLine[11])
            self.clearLineShort(row,155)
            self.screen.addstr(row,155,incomingLine)
        if ('IC 1 T' in incomingLine):
            row = 8 + int(incomingLine[11])
            self.clearLineShort(row,155)
            self.screen.addstr(row,155,incomingLine)
        if ('IC 2 T' in incomingLine):
            row = 12 + int(incomingLine[11])
            self.clearLineShort(row,155)
            self.screen.addstr(row,155,incomingLine)
        if ('IC 3 T' in incomingLine):
            row = 16 + int(incomingLine[11])
            self.clearLineShort(row,155)
            self.screen.addstr(row,155,incomingLine)
        if ('IC 4 T' in incomingLine):
            row = 4 + int(incomingLine[11])
            self.clearLineShort(row,180)
            self.screen.addstr(row,180,incomingLine)
        if ('IC 5 T' in incomingLine):
            row = 8 + int(incomingLine[11])
            self.clearLineShort(row,180)
            self.screen.addstr(row,180,incomingLine)
        if ('IC 6 T' in incomingLine):
            row = 12 + int(incomingLine[11])
            self.clearLineShort(row,180)
            self.screen.addstr(row,180,incomingLine)
        if ('IC 7 T' in incomingLine):
            row = 16 + int(incomingLine[11])
            self.clearLineShort(row,180)
            self.screen.addstr(row,180,incomingLine)
        if ('BAL' in incomingLine): # format: BAL IC# CELL# [ON|OFF]
            row = 22 + int(incomingLine[incomingLine.index('IC') + 2])
            col = 160 + (int(incomingLine[incomingLine.index('CELL') + 4]) * 5)
            self.clearLineBal(row, col)
            if 'ON' in incomingLine:
                self.screen.addstr(row, col, 'BAL')
        self.screen.refresh()

    def clearLine(self, y, x):
        blanks = '                                                  '
        self.screen.addstr(y,x,blanks)

    def clearLineShort(self, y, x):
        blanks = '                      '
        self.screen.addstr(y,x,blanks)

    def clearLineBal(self, y, x):
        blanks = '   '
        self.screen.addstr(y,x,blanks)

def unpack(frame):
    #print("----------------")
    frame = ''.join(char for char in frame if char.isalnum())
    if (len(frame) != 32):
    #     # TODO throw an error up on screen
    #     #print("Malformed frame len " + str(len(frame)) + " encountered - skipping")
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
    cs_calc = chr(checksum >> 8) + " " + chr(checksum & 0xFF)
    cs_rcvd = chr(decoded[14]) + " " + chr(decoded[13])
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
    id = msg[0] #not an endianness problem
    # print("CAN ID:        " + hex(msg[0]).upper()) #at this point, no C3 messages print. JK it happened twice - maybe its a timer thing?
    size = msg[4]
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
        ret.append("INVERTER STATE: " + str(msg[7]))
        ret.append("INVERTER RUN MODE: " + str(msg[9] & 0x1))
        ret.append("INVERTER ACTIVE DISCHARGE STATE: " + str((msg[9] & 0xE0) >> 5))
        ret.append("INVERTER COMMAND MODE: " + str(msg[10]))
        ret.append("INVERTER ENABLE: " + str(msg[11] & 0x1))
        ret.append("INVERTER LOCKOUT: " + str((msg[11] & 0x80) >> 7))
        ret.append("DIRECTION COMMAND: " + str(msg[12]))
    if (id == 0xAB):
        ret.append("POST FAULT LO: 0x" + hex(msg[6]).upper()[2:] + hex(msg[5]).upper()[2:])
        ret.append("POST FAULT HI: 0x" + hex(msg[8]).upper()[2:] + hex(msg[7]).upper()[2:])
        ret.append("RUN FAULT LO: 0x" + hex(msg[10]).upper()[2:] + hex(msg[9]).upper()[2:])
        ret.append("RUN FAULT HI: 0x" + hex(msg[12]).upper()[2:] + hex(msg[11]).upper()[2:])
    if (id == 0xAC):
        ret.append("COMMANDED TORQUE: " + str(b2i16(msg[5:7]) / 10.) + " Nm")
        ret.append("TORQUE FEEDBACK: " + str(b2i16(msg[7:9]) / 10.) + " Nm")
        ret.append("RMS UPTIME: " + str(int(b2ui32(msg[9:13]) * .003)) + " s")
    if (id == 0xC0):
        ret.append("REQUESTED TORQUE: " + str(b2i16(msg[5:7]) / 10.) + " Nm")
        #ret.append("FCU REQUESTED INVERTER ENABLE: " + str(ord(msg[10]) & 0x1))
    if (id == 0xC3):
        ret.append("MCU STATE: " + str(msg[5])) #this stuff no working.
        ret.append("MCU BMS FAULT: " + str(not msg[6] & 0x1))
        ret.append("MCU IMD FAULT: " + str(not (msg[6] & 0x2) >> 1))
        ret.append("MCU INVERTER POWER: " + ("ON" if (((msg[6]) & 0x4) >> 2) == 1 else "OFF"))
        ret.append("MCU SHUTDOWN ABOVE THRESH: " + shutdown_from_flags(msg[6]))
        ret.append("MCU TEMPERATURE: " + str(b2i16(msg[7:9])))
        ret.append("MCU GLV VOLTAGE: " + str(b2ui16(msg[9:11]) / 100.) + " V")
    if (id == 0xC4):
        ret.append("MCU PEDAL ACCEL 1: " + str(b2ui16(msg[5:7])))
        ret.append("MCU PEDAL ACCEL 2: " + str(b2ui16(msg[7:9])))
        ret.append("MCU PEDAL BRAKE: " + str(b2ui16(msg[9:11])))
        ret.append("MCU BRAKE ACT: " + str((msg[12] & 0x4) >> 2))
        ret.append("MCU IMPLAUS ACCEL: " + str(msg[12] & 0x1))
        ret.append("MCU IMPLAUS BRAKE: " + str((msg[12] & 0x2) >> 1))
        ret.append("MCU TORQUE MAP MODE: " + str(msg[13]))
    if (id == 0xCC):
        ret.append("ECU CURRENT: " + str(b2ui16(msg[5:7]) / 100.) + " A")
        ret.append("COOLING CURRENT: " + str(b2ui16(msg[7:9]) / 100.) + " A")
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
        ic = msg[5] & 0xF
        group = (msg[5] & 0xF0) >> 4
        ret.append("IC " + str(ic) + " CELL " + str(group * 3) + ": " + '%.4f' % (b2ui16(msg[6:8]) / 10e3) + " V")
        ret.append("IC " + str(ic) + " CELL " + str(group * 3 + 1) + ": " + '%.4f' % (b2ui16(msg[8:10]) / 10e3) + " V")
        ret.append("IC " + str(ic) + " CELL " + str(group * 3 + 2) + ": " + '%.4f' % (b2ui16(msg[10:12]) / 10e3) + " V")
    if (id == 0xD9):
        ret.append("BMS AVERAGE TEMPERATURE: " + str(b2i16(msg[5:7]) / 100.) + " C")
        ret.append("BMS LOW TEMPERATURE: " + str(b2i16(msg[7:9]) / 100.) + " C")
        ret.append("BMS HIGH TEMPERATURE: " + str(b2i16(msg[9:11]) / 100.) + " C")
    if (id == 0xDA):
        ic = msg[5]
        ret.append("IC " + str(ic) + " THERM 0: " + '%.2f' % (b2ui16(msg[6:8]) / 100.) + " C")
        ret.append("IC " + str(ic) + " THERM 1: " + '%.2f' % (b2ui16(msg[8:10]) / 100.) + " C")
        ret.append("IC " + str(ic) + " THERM 2: " + '%.2f' % (b2ui16(msg[10:12]) / 100.) + " C")
    if (id == 0xDB):
        ret.append("BMS STATE: " + str(msg[5]))
        ret.append("BMS ERROR FLAGS: 0x" + hex(msg[7]).upper()[2:] + hex(msg[6]).upper()[2:])
        ret.append("BMS CURRENT: " + str(b2i16(msg[8:10]) / 100.) + " A")
    if (id == 0xDE):
        bal = "BAL "
        data = b2ui64(msg[5:13])
        group = data & 0x1
        for ic in range(8):
            for cell in range(9):
                bal += "IC" + str(ic + 4 if group == 1 else ic) + " "
                bal += "CELL" + str(cell) + " "
                if (((data >> (0x4 + 0x9 * ic)) & 0x1FF) >> cell) & 0x1 == 1:
                    bal += "ON"
                else:
                    bal += "OFF"
        ret.append(bal)
    if (id == 0xE2):
        ret.append("BMS TOTAL CHARGE: " + str(b2ui32(msg[5:9]) / 10000. + " C"))
        ret.append("BMS TOTAL DISCHARGE: " + str(b2ui32(msg[9:13]) / 10000. + " C"))
    if (id == 0xEA):
        ret.append("TCU WHEEL RPM REAR LEFT: " + str(b2i16(msg[5:7]) / 100.) + " RPM")
        ret.append("TCU WHEEL RPM REAR RIGHT: " + str(b2i16(msg[7:9]) / 100.) + " RPM")
    if (id == 0xEB):
        ret.append("TCU WHEEL RPM FRONT LEFT: " + str(b2i16(msg[5:7]) / 100.) + " RPM")
        ret.append("TCU WHEEL RPM FRONT RIGHT: " + str(b2i16(msg[7:9]) / 100.) + " RPM")
    if (id == 0xED):
        ret.append("TCU DISTANCE TRAVELED: " + str(b2i16(msg[5:7]) / 100.) + " m")    
    return ret

def b2i8(data):
    return struct.unpack("<1b", data[0])[0]

def b2i16(data):
    return struct.unpack("<1h", data[0:2])[0]

def b2ui16(data):
    return struct.unpack("<1H", data[0:2])[0]

def b2ui32(data):
    return struct.unpack("<1I", data[0:4])[0]

def b2ui64(data):
    return struct.unpack("<1Q", data[0:8])[0]

def shutdown_from_flags(flags):
    shutdown = ''

    if ((flags & 0x8) >> 3) == 1:
        shutdown += 'B'
    if ((flags & 0x10) >> 4) == 1:
        shutdown += 'C'
    if ((flags & 0x20) >> 5) == 1:
        shutdown += 'D'
    if ((flags & 0x40) >> 6) == 1:
        shutdown += 'E'
    if ((flags & 0x80) >> 7) == 1:
        shutdown += 'F'

    return shutdown

def fletcher16(data):
    d = data
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


if len(sys.argv) != 2:
    print('Usage:')
    print('telemetry_lte_xbee.py [fcu|mcu]\n')
    quit()

telemClient = TelemetryClient()
curses.wrapper(telemClient.setupScreen)
