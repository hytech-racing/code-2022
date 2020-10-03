import sys
import datetime
import time
import binascii
from cobs import cobs
import codecs
# import msvcrt
import threading
import struct
import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
import requests

MQTT_SERVER = 'localhost'
# MQTT_SERVER = 'ec2-3-134-2-166.us-east-2.compute.amazonaws.com'
# MQTT_SERVER = 'hytech-telemetry.ryangallaway.me'     # MQTT broker hostname

MQTT_PORT = 1883                    # MQTT broker port (non-SSL)
MQTT_TOPIC = 'hytech_car/telemetry'

INFLUX_HOST = 'localhost'
INFLUX_PORT = 8086
INFLUX_DB_NAME = 'hytech_telemetry'

def influx_connect():
    while True:
        print('Attempting to connect to database at {}:{}'.format(INFLUX_HOST, INFLUX_PORT))
        try:
            client = InfluxDBClient(host=INFLUX_HOST, port=INFLUX_PORT)
            db_exists = False
            for db in client.get_list_database():
                if db['name'] == INFLUX_DB_NAME:
                    db_exists = True
                    client.switch_database(INFLUX_DB_NAME)
                    break
            if not db_exists:
                requests.get('http://{}:{}/query?q=CREATE+DATABASE+"{}"'.format(INFLUX_HOST, INFLUX_PORT, INFLUX_DB_NAME))
                client.switch_database(INFLUX_DB_NAME)
            break
        except Exception as e:
            print("Influx connection refused. Trying again in ten seconds.")
            time.sleep(10)
    print("Connected using database {}".format(INFLUX_DB_NAME))
    return client


def mqtt_connect(client, userdata, flags, rc):
    client.subscribe(MQTT_TOPIC)
    print("Subscribed to", MQTT_TOPIC)
    client.publish(MQTT_TOPIC, "Python client connected")

# id_map = [False] * 256;

def mqtt_message(client, userdata, msg):
    timestamp = msg.payload[0:msg.payload.find(b',')]
    frame = msg.payload[msg.payload.find(b',') + 1:-1]
    frame = binascii.hexlify(frame)
    s = ""
    for c in frame:
        s += chr(c)
    #  print(s)
    data = unpack(s)
    # print(data)

    if data != -1:
        timestamp = int(timestamp.decode()) + 946771200 - 68359 #  LTE Module stuck in 1990. Fix if you can.
        json_body = []
        for readout in decode(data):
            if len(str(readout[1])) == 0:
                continue
            if len(readout) == 2:
                json_body.append({
                    "measurement": readout[0],
                    "time": timestamp,
                    "fields": {
                        "value": readout[1]
                    }
                })
            else:
                json_body.append({
                    "measurement": readout[0],
                    "time": timestamp,
                    "fields": {
                        "value": readout[1],
                        "units": readout[2]
                    }
                })
            # if (not id_map[data[0]]):
                # print (hex(data[0]).upper())
                # id_map[data[0]] = True
        print("Writing document: ")
        print(json_body)
        try:
            influx_client.write_points(points=json_body, time_precision='s')
        except Exception as e:
            print("Operation failed. Printing error:")
            print(e)

def live():
    # Set up mqtt connection
    mqtt_client = mqtt.Client()
    mqtt_client.connect(MQTT_SERVER, MQTT_PORT, 60)
    mqtt_client.on_connect = mqtt_connect
    mqtt_client.on_message = mqtt_message
    mqtt_client.loop_start()
    print("Loop start")

    threading.Event().wait()
    # Wait for q to quit
    # input()

    # Time to quit, disconnect MQTT
    # print("Loop stop")
    # client.loop_stop()
    # client.disconnect() # TODO unsure if this should be called

def unpack(frame):
    # print("----------------")
    frame = ''.join(char for char in frame if char.isalnum())
    if (len(frame) != 32):
        # TODO throw an error up on screen
        # print("Malformed frame len " + str(len(frame)) + " encountered - skipping")
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
        print("Decode failed: " + str(e))
        return -1
    # Calculate checksum
    checksum = fletcher16(decoded[0:13])
    cs_calc = chr(checksum >> 8) + " " + chr(checksum & 0xFF)
    cs_rcvd = chr(decoded[14]) + " " + chr(decoded[13])
    if cs_calc != cs_rcvd:
        # print("Decode failed: Checksum mismatch - calc: " + cs_calc + " - rcvd: " + cs_rcvd)
        return -1
    '''out = "Decoded frame: "
    for char in decoded:
        out += binascii.hexlify(char).upper() + " "
    print(out)'''
    return decoded

def decode(msg):
    ret = []
    id = msg[0]
    # id = ord(msg[0])
    # print("CAN ID:        0x" + hex(msg[0]).upper()[2:])
    size = msg[4]
    # size = ord(msg[4])
    #print("MSG LEN:       " + str(size))
    if (id == 0xA0):
        ret.append(["MODULE_A_TEMP",                    (b2i16(msg[5:7]) / 10.),         "C"     ])
        ret.append(["MODULE_B_TEMP",                    (b2i16(msg[7:9]) / 10.),         "C"     ])
        ret.append(["MODULE_C_TEMP",                    (b2i16(msg[9:11]) / 10.),        "C"     ])
        ret.append(["GATE_DRIVER_BOARD_TEMP",           (b2i16(msg[11:13]) / 10.),       "C"     ])
    elif (id == 0xA2):
        ret.append(["RTD_4_TEMP",                       (b2i16(msg[5:7]) / 10.),         "C"     ])
        ret.append(["RTD_5_TEMP",                       (b2i16(msg[7:9]) / 10.),         "C"     ])
        ret.append(["MOTOR_TEMP",                       (b2i16(msg[9:11]) / 10.),        "C"     ])
        ret.append(["TORQUE_SHUDDER",                   (b2i16(msg[11:13]) / 10.),       "Nm"    ])
    elif (id == 0xA5):
        ret.append(["MOTOR_ANGLE",                      (b2i16(msg[5:7]) / 10.)                  ])
        ret.append(["MOTOR_SPEED",                      (b2i16(msg[7:9])),               "RPM"   ])
        ret.append(["ELEC_OUTPUT_FREQ",                 (b2i16(msg[9:11]) / 10.)                ])
        ret.append(["DELTA_RESOLVER_FILT",              b2i16(msg[11:13])                       ])
    elif (id == 0xA6):
        ret.append(["PHASE_A_CURRENT",                  (b2i16(msg[5:7]) / 10.),         "A"     ])
        ret.append(["PHASE_B_CURRENT",                  (b2i16(msg[7:9]) / 10.),         "A"     ])
        ret.append(["PHASE_C_CURRENT",                  (b2i16(msg[9:11]) / 10.),        "A"     ])
        ret.append(["DC_BUS_CURRENT",                   (b2i16(msg[11:13]) / 10.),       "A"     ])
    elif (id == 0xA7):
        ret.append(["DC_BUS_VOLTAGE",                   (b2i16(msg[5:7]) / 10.),         "V"     ])
        ret.append(["OUTPUT_VOLTAGE",                   (b2i16(msg[7:9]) / 10.),         "V"     ])
        ret.append(["PHASE_AB_VOLTAGE",                 (b2i16(msg[9:11]) / 10.),        "V"     ])
        ret.append(["PHASE_BC_VOLTAGE",                 (b2i16(msg[11:13]) / 10.),       "V"     ])
    elif (id == 0xAA):
        ret.append(["VSM_STATE",                        b2ui16(msg[5:7])                        ])
        ret.append(["INVERTER_STATE",                   msg[7]                                  ])
        ret.append(["INVERTER_RUN_MODE",                (msg[9] & 0x1)                          ])
        ret.append(["INVERTER_ACTIVE_DISCHARGE_STATE",  ((msg[9] & 0xE0) >> 5)                  ])
        ret.append(["INVERTER_COMMAND_MODE",            msg[10]                                 ])
        ret.append(["INVERTER_ENABLE",                  (msg[11] & 0x1)                         ])
        ret.append(["INVERTER_LOCKOUT",                 ((msg[11] & 0x80) >> 7)                 ])
        ret.append(["DIRECTION_COMMAND",                msg[12]                                 ])
    elif (id == 0xAB):
        ret.append(["POST_FAULT_LO",                    "0x" + binascii.hexlify(msg[6]).upper() + binascii.hexlify(msg[5]).upper()])
        ret.append(["POST_FAULT_HI",                    "0x" + binascii.hexlify(msg[8]).upper() + binascii.hexlify(msg[7]).upper()])
        ret.append(["RUN_FAULT_LO",                     "0x" + binascii.hexlify(msg[10]).upper() + binascii.hexlify(msg[9]).upper()])
        ret.append(["RUN_FAULT_HI",                     "0x" + binascii.hexlify(msg[12]).upper() + binascii.hexlify(msg[11]).upper()])
    elif (id == 0xAC):
        ret.append(["COMMANDED_TORQUE",                 (b2i16(msg[5:7]) / 10.),         "Nm"    ])
        ret.append(["TORQUE_FEEDBACK",                  (b2i16(msg[7:9]) / 10.),         "Nm"    ])
        ret.append(["RMS_UPTIME",                       int(b2ui32(msg[9:13]) * .003),  "s"     ])
    elif (id == 0xC0):
        ret.append(["REQUESTED_TORQUE",                 (b2i16(msg[5:7]) / 10.),         "Nm"    ])
        #ret.append("FCU REQUESTED INVERTER ENABLE: " + str(ord(msg[10]) & 0x1))
    elif (id == 0xC3):
        ret.append(["MCU_STATE",                        msg[5]                                  ])
        ret.append(["MCU_BMS_FAULT",                    (not msg[6] & 0x1)                      ])
        ret.append(["MCU_IMD_FAULT",                    (not (msg[6] & 0x2) >> 1)               ])
        ret.append(["MCU_INVERTER_POWER",               ("ON" if ((msg[6] & 0x4) >> 2) == 1 else "OFF") ])
        ret.append(["MCU_SHUTDOWN_ABOVE_THRESH",        shutdown_from_flags(msg[6])             ])
        ret.append(["MCU_TEMPERATURE",                  b2i16(msg[7:9])                         ])
        ret.append(["MCU_GLV_VOLTAGE",                  (b2ui16(msg[9:11]) / 100.),"V"           ])
    elif (id == 0xC4):
        ret.append(["MCU_PEDAL_ACCEL_2",                b2ui16(msg[7:9])                        ])
        ret.append(["MCU_PEDAL_BRAKE",                  b2ui16(msg[9:11])                       ])
        ret.append(["MCU_PEDAL_ACCEL_1",                b2ui16(msg[5:7])                        ])
        ret.append(["MCU_BRAKE_ACT",                    ((msg[12] & 0x4) >> 2)                  ])
        ret.append(["MCU_IMPLAUS_ACCEL",                (msg[12] & 0x1)                         ])
        ret.append(["MCU_IMPLAUS_BRAKE",                ((msg[12] & 0x2) >> 1)                  ])
        ret.append(["MCU_TORQUE_MAP_MODE",              msg[13]                                 ])
    elif (id == 0xCC):
        ret.append(["ECU_CURRENT",                      (b2ui16(msg[5:7]) / 100.),"A"            ])
        ret.append(["COOLING_CURRENT",                  (b2ui16(msg[7:9]) / 100.),"A"            ])
    elif (id == 0xD0):
        ret.append(["RCU_STATE",                        msg[5]                                  ])
        ret.append(["RCU_FLAGS",                        "0x" + binascii.hexlify(msg[6]).upper() ])
        ret.append(["GLV_BATT_VOLTAGE",                 (b2ui16(msg[7:9]) / 100.),"V"           ])
        ret.append(["RCU_BMS_FAULT",                    (not msg[6] & 0x1)                      ])
        ret.append(["RCU_IMD_FAULT",                    (not (msg[6] & 0x2) >> 1)               ])
    elif (id == 0xD2):
        ret.append(["FCU_STATE",                        msg[5]                                  ])
        ret.append(["FCU_FLAGS",                        "0x{}".format(binascii.hexlify(msg[6]).upper())])
        ret.append(["FCU_START_BUTTON_ID",              msg[7]                                  ])
        ret.append(["FCU_BRAKE_ACT",                    ((msg[6] & 0x8) >> 3)                    ])
        ret.append(["FCU_IMPLAUS_ACCEL",                (msg[6] & 0x1)                           ])
        ret.append(["FCU_IMPLAUS_BRAKE",                ((msg[6] & 0x4) >> 2)                    ])
    elif (id == 0xD3):
        ret.append(["FCU_PEDAL_ACCEL_1",                 b2ui16(msg[5:7])                        ])
        ret.append(["FCU_PEDAL_ACCEL_2",                 b2ui16(msg[7:9])                        ])
        ret.append(["FCU_PEDAL_BRAKE",                   b2ui16(msg[9:11])                       ])
    elif (id == 0xD7):
        ret.append(["BMS_VOLTAGE_AVERAGE",               (b2ui16(msg[5:7]) / 10e3),      "V"     ])
        ret.append(["BMS_VOLTAGE_LOW",                   (b2ui16(msg[7:9]) / 10e3),      "V"     ])
        ret.append(["BMS_VOLTAGE_HIGH",                  (b2ui16(msg[9:11]) / 10e3),     "V"     ])
        ret.append(["BMS_VOLTAGE_TOTAL",                 (b2ui16(msg[11:13]) / 100.),    "V"     ])
    elif (id == 0xD8):
        ic = "IC_" + str(msg[5] & 0xF) + "_CELL"
        group = ((msg[5] & 0xF0) >> 4) * 3
        ret.append([ic + str(group),                    (b2ui16(msg[6:8]) / 10e3),       "V"     ])
        ret.append([ic + str(group + 1),                (b2ui16(msg[8:10]) / 10e3),      "V"     ])
        ret.append([ic + str(group + 2),                (b2ui16(msg[10:12]) / 10e3),     "V"     ])
    elif (id == 0xD9):
        ret.append(["BMS_AVERAGE_TEMPERATURE",          (b2i16(msg[5:7]) / 100.),       "C"     ])
        ret.append(["BMS_LOW_TEMPERATURE",              (b2i16(msg[7:9]) / 100.),       "C"     ])
        ret.append(["BMS_HIGH_TEMPERATURE",             (b2i16(msg[9:11]) / 100.),      "C"     ])
    elif (id == 0xDA):
        ic = msg[5]
        ret.append(["IC_" + str(ic) + "_THERM 0",        (b2ui16(msg[6:8]) / 100.),       "C"     ])
        ret.append(["IC_" + str(ic) + "_THERM 1",        (b2ui16(msg[8:10]) / 100.),      "C"     ])
        ret.append(["IC_" + str(ic) + "_THERM 2",        (b2ui16(msg[10:12]) / 100.),     "C"     ])
    elif (id == 0xDB):
        ret.append(["BMS_STATE",                        msg[5]                                  ])
        ret.append(["BMS_ERROR_FLAGS",                  "0x" + binascii.hexlify(msg[7]).upper() + binascii.hexlify(msg[6]).upper()])
        ret.append(["BMS_CURRENT",                      (b2i16(msg[8:10]) / 100.),      "A"     ])
    elif (id == 0xDE):
        data = b2ui64(msg[5:13])
        group = data & 0x1
        for ic in range(8):
            for cell in range(9):
                bal = "BAL_IC" + str(ic + 4 if group == 1 else ic) + "_CELL" + str(cell)
                state = ("OFF" if (((data >> (0x4 + 0x9 * ic)) & 0x1FF) >> cell) & 0x1 == 1 else "ON")
                ret.append([bal, state])
    elif (id == 0xE2):
        ret.append("BMS_TOTAL_CHARGE: " + str(b2ui32(msg[5:9]) / 10000. + " C"))
        ret.append("BMS_TOTAL_DISCHARGE: " + str(b2ui32(msg[9:13]) / 10000. + " C"))
    elif (id == 0xEA):
        ret.append(["TCU_WHEEL_RPM_REAR_LEFT",          b2i16(msg[5:7]),                "RPM"  ])
        ret.append(["TCU_WHEEL_RPM_REAR_RIGHT",         b2i16(msg[7:9]),                "RPM"  ])
    elif (id == 0xEB):
        ret.append(["TCU_WHEEL_RPM_FRONT_LEFT",         b2i16(msg[5:7]),                "RPM"  ])
        ret.append(["TCU_WHEEL_RPM_FRONT_RIGHT",        b2i16(msg[7:9]),                "RPM"  ])    return ret

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
    d = data # map(ord,data)
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

influx_client = influx_connect()
live()
