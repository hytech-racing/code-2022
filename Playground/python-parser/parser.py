"""
@Author: Bo Han Zhu
@Date: 1/15/2022
@Description: HyTech custom python parser. Reads CSVs from Raw_Data, parses them, and writes to Parsed_Data

parse_folder --> parse_file --> parse_time
                            --> parse_message --> parse_ID_XXXXXXXXX
"""

import os
from sre_constants import CATEGORY_UNI_NOT_SPACE
import sys
from datetime import datetime

########################################################################
# Custom Parsing Functions Begin
########################################################################

def parse_ID_MC_TEMPERATURES1(raw_message):
    return
def parse_ID_MC_TEMPERATURES2(raw_message):
    return
def parse_ID_MC_TEMPERATURES3(raw_message):
    return
def parse_ID_MC_ANALOG_INPUTS_VOLTAGES(raw_message):
    return
def parse_ID_MC_DIGITAL_INPUTS_STATUS(raw_message):
    return
def parse_ID_MC_MOTOR_POSITION_INFORMATION(raw_message):
    return
def parse_ID_MC_CURRENT_INFORMATION(raw_message):
    return
def parse_ID_MC_VOLTAGE_INFORMATION(raw_message):
    return
def parse_ID_MC_FLUX_INFORMATION(raw_message):
    return
def parse_ID_MC_INTERNAL_VOLTAGES(raw_message):
    return
def parse_ID_MC_INTERNAL_STATES(raw_message):
    return
def parse_ID_MC_FAULT_CODES(raw_message):
    return
def parse_ID_MC_TORQUE_TIMER_INFORMATION(raw_message):
    return
def parse_ID_MC_FLUX_WEAKENING_OUTPUT(raw_message):
    return
def parse_ID_MC_FIRMWARE_INFORMATION(raw_message):
    return
def parse_ID_MC_DIAGNOSTIC_DATA(raw_message):
    return
def parse_ID_MC_COMMAND_MESSAGE(raw_message):
    return
def parse_ID_MC_READ_WRITE_PARAMETER_COMMAND(raw_message):
    return
def parse_ID_MC_READ_WRITE_PARAMETER_RESPONSE(raw_message):
    return
def parse_ID_MCU_STATUS(raw_message):
    return
def parse_ID_MCU_PEDAL_READINGS(raw_message):
    return
def parse_ID_MCU_ANALOG_READINGS(raw_message):
    return
def parse_ID_BMS_ONBOARD_TEMPERATURES(raw_message):
    return
def parse_ID_BMS_ONBOARD_DETAILED_TEMPERATURES(raw_message):
    return
def parse_ID_BMS_VOLTAGES(raw_message):
    return
def parse_ID_BMS_DETAILED_VOLTAGES(raw_message):
    return
def parse_ID_BMS_TEMPERATURES(raw_message):
    return
def parse_ID_BMS_DETAILED_TEMPERATURES(raw_message):
    return
def parse_ID_BMS_STATUS(raw_message):
    return
def parse_ID_FH_WATCHDOG_TEST(raw_message):
    return
def parse_ID_CCU_STATUS(raw_message):
    return
def parse_ID_BMS_BALANCING_STATUS(raw_message):
    return
def parse_ID_BMS_READ_WRITE_PARAMETER_COMMAND(raw_message):
    return
def parse_ID_BMS_PARAMETER_RESPONSE(raw_message):
    return
def parse_ID_BMS_COULOMB_COUNTS(raw_message):
    return
def parse_ID_MCU_GPS_READINGS(raw_message):
    return
def parse_ID_MCU_WHEEL_SPEED(raw_message):
    return
def parse_ID_DASHBOARD_STATUS(raw_message):
    return
def parse_ID_SAB_READINGS_FRONT(raw_message):
    return
def parse_ID_SAB_READINGS_REAR(raw_message):
    return
def parse_ID_EM_STATUS(raw_message):
    return
def parse_ID_EM_MEASUREMENT(raw_message):
    return
def parse_ID_IMU_ACCELEROMETER(raw_message):
    return
def parse_ID_IMU_GYROSCOPE(raw_message):
    return


########################################################################
# Custom Parsing Functions End
########################################################################

def parse_message(raw_id, raw_message):
    if raw_id == "A0": return parse_ID_MC_TEMPERATURES1(raw_message)
    if raw_id == "A1": return parse_ID_MC_TEMPERATURES2(raw_message)
    if raw_id == "A2": return parse_ID_MC_TEMPERATURES3(raw_message)
    if raw_id == "A3": return parse_ID_MC_ANALOG_INPUTS_VOLTAGES(raw_message)
    if raw_id == "A4": return parse_ID_MC_DIGITAL_INPUTS_STATUS(raw_message)
    if raw_id == "A5": return parse_ID_MC_MOTOR_POSITION_INFORMATION(raw_message)
    if raw_id == "A6": return parse_ID_MC_CURRENT_INFORMATION(raw_message)
    if raw_id == "A7": return parse_ID_MC_VOLTAGE_INFORMATION(raw_message)
    if raw_id == "A8": return parse_ID_MC_FLUX_INFORMATION(raw_message)
    if raw_id == "A9": return parse_ID_MC_INTERNAL_VOLTAGES(raw_message)
    if raw_id == "AA": return parse_ID_MC_INTERNAL_STATES(raw_message)
    if raw_id == "AB": return parse_ID_MC_FAULT_CODES(raw_message)
    if raw_id == "AC": return parse_ID_MC_TORQUE_TIMER_INFORMATION(raw_message)
    if raw_id == "AD": return parse_ID_MC_FLUX_WEAKENING_OUTPUT(raw_message)
    if raw_id == "AE": return parse_ID_MC_FIRMWARE_INFORMATION(raw_message)
    if raw_id == "AF": return parse_ID_MC_DIAGNOSTIC_DATA(raw_message)
    
    if raw_id == "C0": return parse_ID_MC_COMMAND_MESSAGE(raw_message)
    if raw_id == "C1": return parse_ID_MC_READ_WRITE_PARAMETER_COMMAND(raw_message)
    if raw_id == "C2": return parse_ID_MC_READ_WRITE_PARAMETER_RESPONSE(raw_message)
    if raw_id == "C3": return parse_ID_MCU_STATUS(raw_message)
    if raw_id == "C4": return parse_ID_MCU_PEDAL_READINGS(raw_message)
    if raw_id == "CC": return parse_ID_MCU_ANALOG_READINGS(raw_message)

    if raw_id == "D5": return parse_ID_BMS_ONBOARD_TEMPERATURES(raw_message)
    if raw_id == "D6": return parse_ID_BMS_ONBOARD_DETAILED_TEMPERATURES(raw_message)
    if raw_id == "D7": return parse_ID_BMS_VOLTAGES(raw_message)
    if raw_id == "D8": return parse_ID_BMS_DETAILED_VOLTAGES(raw_message)
    if raw_id == "D9": return parse_ID_BMS_TEMPERATURES(raw_message)
    if raw_id == "DA": return parse_ID_BMS_DETAILED_TEMPERATURES(raw_message)
    if raw_id == "DB": return parse_ID_BMS_STATUS(raw_message)
    if raw_id == "DC": return parse_ID_FH_WATCHDOG_TEST(raw_message)
    if raw_id == "DD": return parse_ID_CCU_STATUS(raw_message)
    if raw_id == "DE": return parse_ID_BMS_BALANCING_STATUS(raw_message)

    if raw_id == "E0": return parse_ID_BMS_READ_WRITE_PARAMETER_COMMAND(raw_message)
    if raw_id == "E1": return parse_ID_BMS_PARAMETER_RESPONSE(raw_message)
    if raw_id == "E2": return parse_ID_BMS_COULOMB_COUNTS(raw_message)
    if raw_id == "E7": return parse_ID_MCU_GPS_READINGS(raw_message)
    if raw_id == "EA": return parse_ID_MCU_WHEEL_SPEED(raw_message)
    if raw_id == "EB": return parse_ID_DASHBOARD_STATUS(raw_message)
    if raw_id == "EC": return parse_ID_SAB_READINGS_FRONT(raw_message)
    if raw_id == "ED": return parse_ID_SAB_READINGS_REAR(raw_message)

    if raw_id == "100": return parse_ID_EM_STATUS(raw_message)
    if raw_id == "400": return parse_ID_EM_MEASUREMENT(raw_message)
    if raw_id == "470": return parse_ID_IMU_ACCELEROMETER(raw_message)
    if raw_id == "471": return parse_ID_IMU_GYROSCOPE(raw_message)

    # Should not come to here if CAN ID was valid
    print("ERROR: Invalid CAN ID: 0x" + raw_message)
    sys.exit(0)
    #return [raw_id, ["N/A"], [raw_message], [""]]

def parse_time(raw_time):
    '''
    @brief: Converts raw time into human-readable time.
    @input: The raw time given by the raw data CSV.
    @return: A string representing the human-readable time.
    @TODO: add millisecond support
    '''
    raw_time = int(raw_time) / 1000
    raw_time = str(datetime.utcfromtimestamp(raw_time).strftime('%Y-%m-%d %H:%M:%S'))
    return raw_time

def parse_file(filename):
    '''
    @brief: Reads raw data file and creates parsed data CSV.
            Loops through lines to write to parsed datafile.
            Calls the parse_message and parse_time functions as helpers.
    @input: The filename of the raw and parsed CSV.
    @return: N/A
    '''

    infile = open("Raw_Data/" + filename, "r")
    outfile = open("Parsed_Data/" + filename, "w")

    flag_first_line = True
    for line in infile.readlines():
        # On the first line, do not try to parse. Instead, set up the CSV headers.
        if flag_first_line:
            flag_first_line = False
            outfile.write("time,id,message,label,value,unit\n")

        # Otherwise attempt to parse the line.
        else:
            raw_time = line.split(",")[0]
            raw_id = line.split(",")[1]
            length = line.split(",")[2]
            raw_message = line.split(",")[3]

            # Do not parse if the length of the message is 0, otherwise bugs will occur later.
            if length == 0 or raw_message == "\n":
                continue
            
            # Call helper functions
            time = parse_time(raw_time)
            table = parse_message(raw_id, int("0x" + raw_message, 16))

            # Assertions that check for parser failure. Notifies user on where parser broke.
            assert len(table) == 4, "ERROR: Parser expected 4 arguments from parse_message at ID: 0x" + table[0] + ", got: " + str(len(table))
            assert len(table[1]) == len (table[2]) and len(table[1]) == len(table[3]), "ERROR: Label, Data, or Unit numbers mismatch for ID: 0x" + raw_id
            
            # Harvest parsed datafields and write to outfile.
            message = table[0].strip()
            for i in range(len(table[1])):
                label = table[1][i].strip()
                value = str(table[2][i]).strip()
                unit = table[3][i].strip()

                outfile.write(time + ",0x" + raw_id + "," + message + "," + label + "," + value + "," + unit + "\n")

    infile.close()
    outfile.close()
    return

def parse_folder():
    '''
    @brief: Locates Raw_Data directory or else throws errors. Created Parsed_Data directory if not created.
            Calls the parse_file() function on each raw CSV and alerts the user of parsing progress.
    @input: N/A
    @return: N/A
    '''

    # Stop attempting to parse if Raw_Data is not there.
    try:
        directory = os.fsencode("Raw_Data")
    except:
        print("ERROR: Raw_Data folder does not exist. Please move parser.py or create Raw_Data folder.")
        sys.exit(0)

    # Creates Parsed_Data folder if not there.
    if not os.path.exists("Parsed_Data"):
        os.makedirs("Parsed_Data")

    print("Currently parsing, please be patient...")

    # Loops through files and call parse_file on each raw CSV.
    for file in os.listdir(directory):
        filename = os.fsdecode(file)
        if filename.endswith(".CSV") or filename.endswith(".csv"):
            parse_file(filename)
            print("Successfully parsed: " + filename)
        else:
            continue

    return 

########################################################################
# Entry Point to Framework
########################################################################
parse_folder()
print("SUCCESS: Parsing Complete.")