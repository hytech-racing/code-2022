"""
@Author: Bo Han Zhu
@Date: 1/15/2022
@Description: HyTech custom python parser. Reads CSVs from Raw_Data, parses them, and writes to Parsed_Data

parse_folder --> parse_file --> parse_time
                            --> parse_message --> parse_ID_XXXXXXXXX
"""

import os
import sys
from datetime import datetime

def parse_message(raw_id, raw_message):
    return [raw_id, ["N/A"], [raw_message], [""]]

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
            if length == 0:
                continue
            
            # Call helper functions
            time = parse_time(raw_time)
            table = parse_message(raw_id, raw_message)

            # Assertions that check for parser failure. Notifies user on where parser broke.
            assert len(table) == 4, "ERROR: Parser expected 4 arguments from parse_message at ID: 0x" + table[0] + ", got: " + str(len(table))
            assert len(table[1]) == len (table[2]) and len(table[1]) == len(table[3]), "ERROR: Label, Data, or Unit numbers mismatch for ID: 0x" + raw_id
            
            # Harvest parsed datafields and write to outfile.
            message = table[0].strip()
            for i in range(len(table[1])):
                label = table[1][i].strip()
                value = table[2][i].strip()
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

parse_folder() # Entry point to parsing framework
print("SUCCESS: Parsing Complete.")