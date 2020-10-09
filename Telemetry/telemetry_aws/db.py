from influxdb import InfluxDBClient
import requests
import struct
import binascii
from cobs import cobs
import threading

INFLUX_HOST = 'localhost'
INFLUX_PORT = 8086

class DB:
    def __init__(self, INFLUX_DB_NAME, time_precision):
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
        self.influx_client = client
        self.time_precision = time_precision
        self.json_body = []
        self.writing = False

    def write(self, timestamp, data):
        for readout in decode(data):
            if len(str(readout[1])) == 0:
                continue
            if len(readout) == 2:
                self.json_body.append({
                    "measurement": readout[0],
                    "time": timestamp,
                    "fields": {
                        "value": readout[1]
                    }
                })
            else:
                self.json_body.append({
                    "measurement": readout[0],
                    "time": timestamp,
                    "fields": {
                        "value": readout[1],
                        "units": readout[2]
                    }
                })
        # print("Writing document: ")
        # print(json_body)
        if not self.writing:
            try:
                threading.Thread(target=self.buffered_write).start()
            except Exception as e:
                print(e)
        
    def buffered_write(self):
        json = self.json_body
        self.json_body = []
        self.writing = True
        
        try:
            self.influx_client.write_points(points=json, time_precision=self.time_precision)
            self.writing = False
        except Exception as e:
            print("Operation failed. Printing error:")
            print(e)

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
        ret.append(["POST_FAULT_LO", "0x" + hex(msg[6]).upper()[2:] + hex(msg[5]).upper()[2:]])
        ret.append(["POST_FAULT_HI", "0x" + hex(msg[8]).upper()[2:] + hex(msg[7]).upper()[2:]])
        ret.append(["RUN_FAULT_LO", "0x" + hex(msg[10]).upper()[2:] + hex(msg[9]).upper()[2:]])
        ret.append(["RUN_FAULT_HI", "0x" + hex(msg[12]).upper()[2:] + hex(msg[11]).upper()[2:]])
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
        ret.append(["MCU_PEDAL_ACCEL_1",                b2ui16(msg[5:7])                        ])
        ret.append(["MCU_PEDAL_ACCEL_2",                b2ui16(msg[7:9])                        ])
        ret.append(["MCU_PEDAL_BRAKE",                  b2ui16(msg[9:11])                       ])
        ret.append(["MCU_IMPLAUS_ACCEL",                (msg[11] & 0x1)                         ])
        ret.append(["MCU_IMPLAUS_BRAKE",                ((msg[11] & 0x2) >> 1)                  ])
        ret.append(["MCU_BRAKE_ACT",                    ((msg[11] & 0x4) >> 2)                  ])
        ret.append(["MCU_TORQUE_MAP_MODE",              msg[12]                                 ])
    elif (id == 0xCC):
        ret.append(["ECU_CURRENT",                      (b2ui16(msg[5:7]) / 100.),"A"            ])
        ret.append(["COOLING_CURRENT",                  (b2ui16(msg[7:9]) / 100.),"A"            ])
    elif (id == 0xD0):
        ret.append(["RCU_STATE",                        msg[5]                                  ])
        ret.append(["RCU_FLAGS",                        "0x" + hex(msg[6]).upper()[2:]          ])
        ret.append(["GLV_BATT_VOLTAGE",                 (b2ui16(msg[7:9]) / 100.),"V"           ])
        ret.append(["RCU_BMS_FAULT",                    (not msg[6] & 0x1)                      ])
        ret.append(["RCU_IMD_FAULT",                    (not (msg[6] & 0x2) >> 1)               ])
    elif (id == 0xD2):
        ret.append(["FCU_STATE",                        msg[5]                                  ])
        ret.append(["FCU_FLAGS",                        "0x{}".format(hex(msg[6]).upper()[2:])  ])
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
        ic = "IC_" + str(msg[5] & 0xF) + "_CELL_"
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
        ret.append(["IC_" + str(ic) + "_THERM_0",        (b2ui16(msg[6:8]) / 100.),       "C"     ])
        ret.append(["IC_" + str(ic) + "_THERM_1",        (b2ui16(msg[8:10]) / 100.),      "C"     ])
        ret.append(["IC_" + str(ic) + "_THERM_2",        (b2ui16(msg[10:12]) / 100.),     "C"     ])
    elif (id == 0xDB):
        ret.append(["BMS_STATE",                        msg[5]                                  ])
        ret.append(["BMS_ERROR_FLAGS",                  "0x" + hex(msg[7]).upper()[2:] + hex(msg[6]).upper()[2:] ])
        ret.append(["BMS_CURRENT",                      (b2i16(msg[8:10]) / 100.),      "A"     ])
    elif (id == 0xDE):
        data = b2ui64(msg[5:13])
        group = data & 0x1
        for ic in range(8):
            for cell in range(9):
                bal = "BAL_IC" + str(ic + 4 if group == 1 else ic) + "_CELL" + str(cell)
                state = ("ON" if (((data >> (0x4 + 0x9 * ic)) & 0x1FF) >> cell) & 0x1 == 1 else "OFF")
                ret.append([bal, state])
    elif (id == 0xE2):
        ret.append(["BMS_TOTAL_CHARGE",                 b2ui32(msg[5:9]) / 10000.,      "C"    ])
        ret.append(["BMS_TOTAL_DISCHARGE",              b2ui32(msg[9:13]) / 10000.,     "C"    ])
    elif (id == 0xE7):
        ret.append(["LATITUDE",                         b2i32(msg[5:9]) / 10000.               ])
        ret.append(["LONGITUDE",                        b2i32(msg[9:13]) / 10000.              ])
    elif (id == 0xE8):
        ret.append(["ALTITUDE",                         b2i32(msg[5:9]) / 10000.               ])
        ret.append(["SPEED",                            b2i32(msg[9:13]) / 10000.              ])
    elif (id == 0xE9):
        ret.append(["GPS_FIX_QUALITY",                  b2ui8(msg[5:6])                        ])
        ret.append(["GPS_SATELLITE_COUNT",              b2ui8(msg[6:7])                        ])
        ret.append(["TIMESTAMP_SECONDS",                b2i16(msg[7:11])                       ])
        ret.append(["TIMESTAMP_MILLISECONDS",           b2i16(msg[11:13])                      ])
    elif (id == 0xEA):
        ret.append(["TCU_WHEEL_RPM_REAR_LEFT",          b2i16(msg[5:7]) / 100,          "RPM"  ])
        ret.append(["TCU_WHEEL_RPM_REAR_RIGHT",         b2i16(msg[7:9]) / 100,          "RPM"  ])
    elif (id == 0xEB):
        ret.append(["TCU_WHEEL_RPM_FRONT_LEFT",         b2i16(msg[5:7]) / 100,          "RPM"  ])
        ret.append(["TCU_WHEEL_RPM_FRONT_RIGHT",        b2i16(msg[7:9]) / 100,          "RPM"  ])
    elif (id == 0xEC):
        ret.append(["MCU_SLIP_RATIO",                   b2i16(msg[5:7]) / 100.                 ])
        ret.append(["MCU_SLIP_LIMITING_FACTOR",         b2i16(msg[7:9]) / 100.                 ])
    elif (id == 0xED):
        ret.append(["TCU_DISTANCE_TRAVELED",            b2i16(msg[5:7]) / 100.,         "m"    ])

    return ret

def b2i8(data):
    return struct.unpack("<1b", data[0:1])[0]

def b2ui8(data):
    return struct.unpack("<1B", data[0:1])[0]

def b2i16(data):
    return struct.unpack("<1h", data[0:2])[0]

def b2ui16(data):
    return struct.unpack("<1H", data[0:2])[0]

def b2i32(data):
    return struct.unpack("<1i", data[0:4])[0]

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
