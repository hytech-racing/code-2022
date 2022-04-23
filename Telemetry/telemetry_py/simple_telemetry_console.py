'''
@Author: Sophia Smith, Bo Han Zhu
@Date: 4/23/2022
@Description: HyTech custom python parser functions.
@TODO: Extend UI to current live console display messages
@TODO: Create new thread for incoming live messages instead of reading from CSV
'''


import PySimpleGUI as sg, sys
import threading
import time
from os import path

__file__ = sys.path[0]
sys.path.insert(1, "../telemetry_parsers")
from parser_functions import parse_message

DICT = {
    "RMS_INVERTER" : {
        "MODULE_A_TEMPERATURE": "N/A",
        "MODULE_B_TEMPERATURE": "N/A",
        "MODULE_C_TEMPERATURE": "N/A"
    },
    "BATTERY_MANAGEMENT_SYSTEM": {
        "BMS_AVERAGE_TEMPERATURE": "N/A",
        "BMS_LOW_TEMPERATURE": "N/A",
        "BMS_HIGH_TEMPERATURE": "N/A"
    }
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
@brief: Thread to read raw CSV line, parse it, and send event followed by pausing for 100 ms to GUI if match 
        Sends event to close GUI upon CSV read completion
        Requires a raw data CSV in the current directory with the name raw_data.csv
@param[in]: window - the PySimpleGUI window obect
'''
def read_from_csv_thread(window):
    infile = open("raw_data.csv", "r")
    line_count =  1 # bypass first header line
    raw_data_lines = infile.readlines()

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
                    window.write_event_value("-Update Data-", [name, name + ":" + data + " " + units])
                    time.sleep(0.1)

        line_count += 1

    window.write_event_value("-Read CSV Done-", "No data for you left")

'''
@brief: The main function to spawn the PySimpleGUI and handle events
'''
def main():
    sg.change_look_and_feel("Dark")
    rms = [[sg.Text("RMS INVERTER", font="Any 15")]]
    battery = [[sg.Text("BATTERY MANAGEMENT SYSTEM", font="Any 15")]]
    
    for label, value in DICT["RMS_INVERTER"].items():
        rms.append([sg.Text(label+": " + value, justification="left", size=(35,1), key=label)])
    for label, value in DICT["BATTERY_MANAGEMENT_SYSTEM"].items():
        battery.append([sg.Text(label+": " + value, justification="left", size=(35,1), key=label)])

    layout = [[sg.Column(rms), sg.Column(battery)]]

    window = sg.Window("Live Telemetry Console",size=(800, 400), resizable=True).Layout(layout)

    thread = threading.Thread(target=read_from_csv_thread, args=[window], daemon=True)
    thread.start()

    # Event Loop
    while True:
        event, values = window.read(timeout=100)

        if event in (sg.WIN_CLOSED, "Exit"):
            break
        elif event == "-Read CSV Done-":
            thread.join(timeout=0)
            break
        elif event == "-Update Data-":
            window[values["-Update Data-"][0]].update(values["-Update Data-"][1])
            window.refresh()

    window.close()

############################
# Entry point to application
############################
main()
