import PySimpleGUI as sg, sys
import os
import json
from os import path
__file__ = sys.path[0]

DATA_FILE = os.path.join(__file__, 'data.json')
DATA_KEYS_TO_ELEMENT_KEYS = {'MODULE_A_TEMPERATURE': '-MODULE A TEMPERATURE-', 'MODULE_B_TEMPERATURE': '-MODULE B TEMPERATURE-','MODULE_C_TEMPERATURE': '-MODULE C TEMPERATURE-'}

def main():
    window, data = None, load_data(DATA_FILE)

    while True:             # Event Loop
        if window is None:
            window = create_main_window(data)

        event, values = window.read()

        if event in (sg.WIN_CLOSED, 'Exit'):
            break
        if event in ('Update Data', 'Data'):
            event, values = create_main_window(data).read(close=True)
            if event == 'Save':
                window.close()
                window = None
                save_settings(DATA_FILE, data, values)
    window.close()

def load_data(data_files):
    try:
        with open(data_files, 'r') as f:
            data1 = json.load(f)
    except Exception as e:
        print('Failed to load data...')
    return data1

def get_keys(dl, keys=None):
    keys = keys or []
    if isinstance(dl, dict):
        keys += dl.keys()
        _ = [get_keys(x, keys) for x in dl.values()]
    elif isinstance(dl, list):
        _ = [get_keys(x, keys) for x in dl]
    return list(set(keys))

def create_main_window(data):
    sg.change_look_and_feel('Dark')
    form = sg.FlexForm("Live Telemetry Console")

    def TextLabel(text): return sg.Text(text+':',justification='left', size=(50,1))
    def TextLabel1(text,value): return sg.Text(text+': ' + value, justification='left', size=(35,1))
    def DataLabel(text,value): return [TextLabel1(text,value)]

    rms_kv, battery_kv = {}, {}
    rms= [[sg.Text('RMS INVERTER', font='Any 15')]]
    battery = [[sg.Text('BATTERY_MANAGEMENT_SYSTEM', font='Any 15')]]

    for key, value in data['RMS_INVERTER'].items():
        rms_kv[key] = value
        rms.append(DataLabel(key,str(value)))


    for key, value in data['BATTERY_MANAGEMENT_SYSTEM'].items():
        battery_kv[key] = value
        battery.append(DataLabel(key,str(value)))

    print(rms)

    column1 = sg.Column(rms)
    column2 = sg.Column(battery)
    layout = [[column1, column2]]

    window = sg.Window('Live Telemetry Console',size=(800, 400), resizable=True).Layout(layout)
    window.Read()

    return window



main()
