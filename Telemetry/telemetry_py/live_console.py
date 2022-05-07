'''
@Author: Sophia Smith, Bo Han Zhu
@Date: 4/23/2022
@Description: HyTech live telemetry console
@TODO: Extend UI to current live console display messages
'''


import PySimpleGUI as sg
import threading
from os import path
from enum import Enum
import paho.mqtt.client as mqtt
import binascii
import struct
import sys

__file__ = sys.path[0]
sys.path.insert(1, "../telemetry_parsers")
from parser_functions import parse_message
sys.path.insert(1, '../telemetry_aws')
from db import unpack

# Connection type definitions
class ConnectionType(Enum):
    SERVER = 0
    ESP32 = 1
    TEST_CSV = 2
    UNKNOWN = 3

# AWS/MQTT Connection Definitions
MQTT_SERVER = "ec2-3-134-2-166.us-east-2.compute.amazonaws.com"
MQTT_PORT   = 1883
MQTT_TOPIC  = 'hytech_car/telemetry'

# Set this to whatever the script is running
# @TODO: Make a screen at the beginning to let the user choose the type
CONNECTION = ConnectionType.SERVER.value

DICT = {
    "RMS_INVERTER" : {
        "MODULE_A_TEMPERATURE": " ",
        "MODULE_B_TEMPERATURE": " ",
        "MODULE_C_TEMPERATURE": " ",
        "GATE_DRIVER_BOARD_TEMPERATURE": " ",
        "RTD_4_TEMPERATURE": " ",
        "RTD_5_TEMPERATURE": " ",
        "MOTOR_TEMPERATURE": " ",
        "TORQUE_SHUDDER": " ",
        "MOTOR_ANGLE": " ",
        "MOTOR_SPEED": " ",
        "ELEC_OUTPUT_FREQ": " ",
        "DELTA_RESOLVER_FILTERED": " ",
        "PHASE_A_CURRENT": " ",
        "PHASE_B_CURRENT": " ",
        "PHASE_C_CURRENT": " ",
        "DC_BUS_CURRENT": " ",
        "DC_BUS_VOLTAGE": " ",
        "OUTPUT_VOLTAGE": " ",
        "PHASE_AB_VOLTAGE": " ",
        "PHASE_BC_VOLTAGE": " ",
        "VSM_STATE": " ",
        "INVERTER_STATE": " ",
        "INVERTER_RUN_MODE": " ",
        "INVERTER_ACTIVE_DISCHARGE_STATE": " ",
        "INVERTER_COMMAND_MODE": " ",
        "INVERTER_ENABLE_STATE": " ",
        "INVERTER_ENABLE_LOCKOUT": " ",
        "DIRECTION_COMMAND": " ",
        "POST_FAULT_LO": " ",
        "POST_FAULT_HI": " ",
        "RUN_FAULT_LO": " ",
        "RUN_FAULT_HI": " ",
        "COMMANDED_TORQUE": " ",
        "TORQUE_FEEDBACK": " ",
        "RMS_UPTIME": " "
    },
    "BATTERY_MANAGEMENT_SYSTEM": {
        "BMS_AVERAGE_TEMPERATURE": " ",
        "BMS_LOW_TEMPERATURE": " ",
        "BMS_HIGH_TEMPERATURE": " ",
        "BMS_STATE": " ",
        "BMS_ERROR_FLAGS": " ",
        "BMS_CURRENT": " ",
        "BMS_VOLTAGE_AVERAGE": " ",
        "BMS_VOLTAGE_LOW": " ",
        "BMS_VOLTAGE_HIGH": " ",
        "BMS_VOLTAGE_TOTAL": " ",
        "BMS_TOTAL_CHARGE": " ",
        "BMS_TOTAL_DISCHARGE": " "
    },
    "MAIN_ECU": {
        "IMD_OK_HIGH": " ",
        "BMS_OK_HIGH": " ",
        "BSPD_OK_HIGH": " ",
        "SOFTWARE_OK_HIGH": " ",
        "SHUTDOWN_D_ABOVE_THRESHOLD": " ",
        "SHUTDOWN_E_ABOVE_THRESHOLD": " ",
        "INVERTER_POWERED": " ",
        "ACCELERATOR_PEDAL_1": " ",
        "ACCELERATOR_PEDAL_2": " ",
        "BRAKE_TRANSDUCER_1": " ",
        "BRAKE_TRANSDUCER_2": " ",
        "BRAKE_PEDAL_ACTIVE": " ",
        "NO_ACCEL_IMPLAUSIBILITY": " ",
        "NO_BRAKE_IMPLAUSIBILITY": " ",
        "TORQUE_MODE": " ",
        "MAX_TORQUE": " ",
        "REQUESTED_TORQUE": " "
    },
    "GLV_CURRENT_READINGS": {
        "ECU_CURRENT": " ",
        "COOLING_CURRENT": " ",
        "TEMPERATURE": " ",
        "GLV_BATTERY_VOLTAGE": " "
    },
    "WHEEL_SPEED_SENSORS": {
        "RPM_BACK_LEFT": " ",
        "RPM_BACK_RIGHT": " ",
        "RPM_FRONT_LEFT": " ",
        "RPM_FRONT_RIGHT": " "
    },
    "SENSOR_ACQUISITION_BOARD": {
        "FL_SUSP_LIN_POT": " ",
        "FR_SUSP_LIN_POT": " ",
        "BL_SUSP_LIN_POT": " ",
        "BR_SUSP_LIN_POT": " ",
        "COOLING_LOOP_FLUID_TEMP": " ",
        "AMB_AIR_TEMP": " "
    },
    "RACEGRADE_IMU": {
        "LAT_ACCEL": " ",
        "LONG_ACCEL": " ",
        "VERT_ACCEL": " ",
        "YAW": " ",
        "PITCH": " ",
        "ROLL": " "
    },
    "ENERGY_METER": {
        "CURRENT": " ",
        "VOLTAGE": " ",
        "CURRENT_GAIN": " ",
        "VOLTAGE_GAIN": " ",
        "OVERVOLTAGE": " ",
        "OVERPOWER": " ",
        "LOGGING": " "
    },
    "DASHBOARD": {
        "SSOK_ABOVE_THRESHOLD": " ",
        "SHUTDOWN_H_ABOVE_THRESHOLD": " "
    },
}

'''
@brief: Helper function to search for keys in a nested dictionary
@reference: https://stackoverflow.com/questions/49662970/how-can-i-search-for-specific-keys-in-this-nested-dictionary-in-python
@param[in]: k - the key to search
@param[in]: d - the (nested) dictionary
@param[out]: true if found, false if not
'''
def recursive_lookup(k, d):
    if k in d: return d[k]
    for v in d.values():
        if isinstance(v, dict):
            a = recursive_lookup(k, v)
            if a is not None: return a
    return None

'''
@brief: Thread to read raw CSV line, parse it, and send event to GUI if match 
        Sends event to close GUI upon CSV read completion
        Requires a raw data CSV in the current directory with the name raw_data.csv
@param[in]: window - the PySimpleGUI window object
'''
def read_from_csv_thread(window):
    infile = open("raw_data.csv", "r")
    line_count =  1 # bypass first header line
    raw_data_lines = infile.readlines()
    window.write_event_value("-Test Connection Success-", "good job!")

    while line_count < len(raw_data_lines):
        raw_id = raw_data_lines[line_count].split(",")[1]
        length = raw_data_lines[line_count].split(",")[2]
        raw_message = raw_data_lines[line_count].split(",")[3]
        raw_message = raw_message[:(int(length) * 2)] # Strip trailing end of line/file characters that may cause bad parsing
        raw_message = raw_message.zfill(16) # Sometimes messages come truncated if 0s on the left. Append 0s so field-width is 16.
        table = parse_message(raw_id, raw_message)
        
        if table != "INVALID_ID" and table != "UNPARSEABLE":
            for i in range(len(table[1])):
                name = table[1][i].upper()
                data = str(table[2][i])
                units = table[3][i]
                if recursive_lookup(name, DICT):
                    window.write_event_value("-Update Data-", [name, name.replace("_", " ") + ": " + data + " " + units])

        line_count += 1

    window.write_event_value("-Read CSV Done-", "No data for you left")

'''
@brief: Thread to connect to MQTT broker on AWS EC2 instance.
        Parses incoming messages and packages them as an event to the GUI if match.
@param[in]: window - the PySimpleGUI window object
'''
def mqtt_connection_thread(window):
    ##############################
    # Paho MQTT Callback Functions
    ##############################

    '''
    @brief: Callback function to handle MQTT messages. Runs them through the parser if valid and sends update event to GUI.
    @param[in]: client - unused
    @param[in]: userdata - unused
    @param[in]: msg - the raw hexlified MQTT message
    '''
    def mqtt_message(client, userdata, msg):
        frame = msg.payload[msg.payload.find(b',') + 1:-1]
        frame = binascii.hexlify(frame)
        data = unpack("".join(chr(c) for c in frame))

        if data != -1:
            id = format(data[0], 'x').upper()
            size = data[4]
            raw = format(struct.unpack(">1Q", data[5:13])[0], 'x')[:size*2].zfill(16)

            table = parse_message(id, raw)
            if table != "INVALID_ID" and table != "UNPARSEABLE":
                for i in range(len(table[1])):
                    name = table[1][i].upper()
                    data = str(table[2][i])
                    units = table[3][i]
                    if recursive_lookup(name, DICT):
                        window.write_event_value("-Update Data-", [name, name.replace("_", " ") + ": " + data + " " + units])

    '''
    @brief: Callback function for MQTT connection success. Sends an connection succes event to the GUI.
    @param[in]: client - the MQTT client object
    @param[in]: userdata - unused
    @param[in]: flags - unused
    @param[in]: rc - unused
    '''
    def mqtt_connect(client, userdata, flags, rc):
        window.write_event_value("-Connection Success-", "good job!")
        client.subscribe(MQTT_TOPIC)

    ################################
    # Paho MQTT Setup and Connection
    ################################
    client = mqtt.Client()
    client.on_connect = mqtt_connect
    client.on_message = mqtt_message
    client.connect(MQTT_SERVER, MQTT_PORT, 60)
    client.loop_forever()

'''
@brief: The main function to spawn the PySimpleGUI and handle events
'''
def main():
    sg.change_look_and_feel("Black")
    title_font = ("Courier New", 12)
    text_font = ("Courier New", 8)

    rms = [[sg.Text("RMS INVERTER", pad=(0,2), font=title_font)]]
    dashboard = [[sg.Text("DASHBOARD", pad=(0,2), font=title_font)]]
    bms = [[sg.Text("BATTERY MANAGEMENT SYSTEM", pad=(0,2), font=title_font)]]
    main_ecu = [[sg.Text("MAIN ECU", pad=(0,2), font=title_font)]]
    glv_current_readings = [[sg.Text("GLV CURRENT READINGS", pad=(0,2), font=title_font)]]
    wheel_speed_sensors = [[sg.Text("WHEEL SPEED SENSORS", pad=(0,2), font=title_font)]]
    sab = [[sg.Text("SENSOR ACQUISITION BOARD", pad=(0,2), font=title_font)]]
    imu = [[sg.Text("RACEGRADE IMU", pad=(0,2), font=title_font)]]
    em = [[sg.Text("ENERGY METER", pad=(0,2), font=title_font)]]
    
    
    for label, value in DICT["RMS_INVERTER"].items():
        rms.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])
    for label, value in DICT["BATTERY_MANAGEMENT_SYSTEM"].items():
        bms.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])
    for label, value in DICT["MAIN_ECU"].items():
        main_ecu.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])
    for label, value in DICT["DASHBOARD"].items():
        dashboard.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])
    for label, value in DICT["GLV_CURRENT_READINGS"].items():
        glv_current_readings.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])
    for label, value in DICT["WHEEL_SPEED_SENSORS"].items():
        wheel_speed_sensors.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])
    for label, value in DICT["SENSOR_ACQUISITION_BOARD"].items():
        sab.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])
    for label, value in DICT["RACEGRADE_IMU"].items():
        imu.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])
    for label, value in DICT["ENERGY_METER"].items():
        em.append([sg.Text(label.replace("_", " ") + ": " + value, justification="left", size=(40,1), pad=(0,0), font=text_font, key=label)])

    connection_text = sg.Text("CONSOLE STATUS: NOT CONNECTED", justification="left", pad=(5,10), text_color='red', font=title_font, key="-Connection Text-")
    column1 = sg.Column(rms, vertical_alignment='t')
    column2 = sg.Column(bms + [[sg.Text(" ", size=(40,1), pad=(0,0), font=text_font)]] + main_ecu + [[sg.Text(" ", size=(40,1), pad=(0,0), font=text_font)]] + glv_current_readings, vertical_alignment='t')
    column3 = sg.Column(dashboard + [[sg.Text(" ", size=(40,1), pad=(0,0), font=text_font)]] + wheel_speed_sensors + [[sg.Text(" ", size=(40,1), pad=(0,0), font=text_font)]] + sab + [[sg.Text(" ", size=(40,1), pad=(0,0), font=text_font)]] + imu + [[sg.Text(" ", size=(40,1), pad=(0,0), font=text_font)]] + em, vertical_alignment='t')

    layout = [[connection_text], [column1, column2, column3]]

    window = sg.Window("HyTech Racing Live Telemetry Console", resizable=True).Layout(layout).Finalize()
    window.Maximize()

    # Choose messaging thread based on connection type
    if CONNECTION == ConnectionType.SERVER.value:
        thread = threading.Thread(target=mqtt_connection_thread, args=[window], daemon=True)
    elif CONNECTION == ConnectionType.ESP32.value:
        sys.exit("ESP32 connection type currently not implemented. Terminating script")
    elif CONNECTION == ConnectionType.TEST_CSV.value:
        thread = threading.Thread(target=read_from_csv_thread, args=[window], daemon=True)
    else:
        sys.exit("Invalid connection source selection. Terminating script")

    thread.start()

    # Event Loop
    while True:
        event, values = window.read()

        if event in (sg.WIN_CLOSED, "Quit"):
            break
        elif event == "-Read CSV Done-":
            thread.join(timeout=0)
            break
        elif event == "-Test Connection Success-":
            window["-Connection Text-"].update("CONSOLE STATUS: TESTING", text_color="yellow")
        elif event == "-Connection Success-":
            window["-Connection Text-"].update("CONSOLE STATUS: CONNECTED", text_color="green")
        elif event == "-Update Data-":
            window[values["-Update Data-"][0]].update(values["-Update Data-"][1])
            window.refresh()

    window.close()

############################
# Entry point to application
############################
main()
