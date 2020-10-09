import curses
import sys
import datetime
import binascii
import paho.mqtt.client as mqtt

sys.path.insert(1, '../telemetry_aws')
from db import decode, unpack, fletcher16
from console_config import get_config

MQTT_SERVER = 'localhost'
# MQTT_SERVER = 'ec2-3-134-2-166.us-east-2.compute.amazonaws.com'
# MQTT_SERVER = 'hytech-telemetry.ryangallaway.me'     # MQTT broker hostname

MQTT_PORT   = 1883                    # MQTT broker port (non-SSL)
MQTT_TOPIC  = 'hytech_car/telemetry'

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
            # if data[0] == 0xEC:
            #     self.screen.addstr(0,59, str(b2i16(data[5:7]) / 100.))

            lines = decode(data)

            with open(self.filename, "a") as f:
                # TODO update to use timestamp sent over LTE instead of local timestamp
                timestamp = str(datetime.datetime.now())

                for line in lines:
                    key = line[0].replace('_', ' ')
                    line = key + ': ' + str(line[1]) + ((" " + line[2]) if len(line) > 2 else "")
                    f.write(timestamp + ' ' + line + '\n')

                    if data[0] == 0xD8:
                        row = 10 * (int(key[3]) % 4) + int(key[10]) + 4
                        col = 105 + 25 * (int(key[3]) // 4)
                        self.clearLineShort(row, col)
                        self.screen.addstr(row, col, line)
                    elif data[0] == 0xDA:
                        row = 4 * (int(key[3]) % 4 + 1) + int(key[11])
                        col = 155 + 25 * (int(key[3]) // 4)
                        self.clearLineShort(row, col)
                        self.screen.addstr(row, col, line)
                    elif data[0] == 0xDE:
                        row = 22 + int(key[6])
                        col = 160 + 5 * int(key[12])
                        self.clearLineBal(row, col)
                        if "ON" in line:
                            self.screen.addstr(row, col, "BAL")
                    else:
                        [row, col] = self.config[key]
                        self.clearLine(row,col)
                        self.screen.addstr(row, col, line)
        else:
            self.countBadFrames += 1

    def setupScreen(self, initScreen):
        self.screen = initScreen
        self.config = get_config(sys.argv[1])

        if sys.argv[1] == 'fcu':
            self.ecu_version = 0
        else:
            self.ecu_version = 1

        self.screen.border('#', '#', '#', '#', 0, 0, 0, 0)
        self.screen.addstr(0,5,'HYTECH RACING 2019 VEHICLE SERIAL DEBUGGER')

        self.screen.addstr(0,5,'HYTECH RACING 2019 VEHICLE SERIAL DEBUGGER')
        self.screen.addstr(3,5,'RMS INVERTER')
        self.screen.addstr(40,5,'GLV CURRENT READINGS')
        self.screen.addstr(3,55,'BATTERY MANAGEMENT SYSTEM')
        self.screen.addstr(3,105,'BATTERY MANAGEMENT SYSTEM DETAILED VOLTAGES')
        self.screen.addstr(3,155,'BATTERY MANAGEMENT SYSTEM DETAILED TEMPERATURES')

        if self.ecu_version == 0:
            self.screen.addstr(17,55,'FRONT CONTROL UNIT')
            self.screen.addstr(30,55,'REAR CONTROL UNIT')
        else:
            self.screen.addstr(17,55,'MAIN CONTROL UNIT')

        for key, pos in self.config.items():
            self.screen.addstr(pos[0], pos[1], key + ":")

        self.screen.addstr(20,155,'BATTERY MANAGEMENT SYSTEM BALANCING STATUS')
        for i in range(8):
            self.screen.addstr(21, 160 + 5 * i, 'C' + str(i))
        for i in range(7):
            self.screen.addstr(22 + i, 155, 'IC' + str(i))

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

    def clearLine(self, y, x):
        blanks = '                                                  '
        self.screen.addstr(y,x,blanks)

    def clearLineShort(self, y, x):
        blanks = '                      '
        self.screen.addstr(y,x,blanks)

    def clearLineBal(self, y, x):
        blanks = '   '
        self.screen.addstr(y,x,blanks)

if len(sys.argv) != 2:
    print('Usage:')
    print('telemetry_lte_xbee.py [fcu|mcu]\n')
    quit()

telemClient = TelemetryClient()
curses.wrapper(telemClient.setupScreen)
