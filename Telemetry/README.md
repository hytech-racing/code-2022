![HyTech Logo](https://hytechracing.gatech.edu/images/hytech_logo_small.png)
# HyTech Custom Parsers
## Intro
Welcome to the folder containing all the HyTech Racing data acquisition services. The entire system is Python and MatLab based.

This guide is split into two parts: a **User's Guide** and a **Developer's Guide**:
- If you will be working on these files (i.e. you are a data aq member), you will need to refer to the **Developer's Guide**
- Otherwise, the **User's Guide** should suffice for everyone else

_If you have any questions or need troubleshooting, feel free to reach out to Bo Han Zhu on Teams or via email: bohanzhu9@gatech.edu_

## Setup
1. Python 3 is required, and may be installed here: https://www.python.org/downloads/. Make sure it is added to PATH
2. For MatLab, you will need an education/work license to get it for free. Follow GT's instructions here: https://www.matlab.gatech.edu/
    - Really the only thing you need MatLab for is to plot the data after parsing. Otherwise, just having Python is enough
3. If you have not done so already, clone the overarching GitHub repo (e.g. code-2022) or download it as a zip, extract, and save to somewhere safe
4. Change directory to here. For example, if you are in code-2022, all you have to do is `cd Telemetry`
5. Once you are here, download the needed pip libraries by issuing the command `pip install -r requirements.txt`

## User's Guide
If you are a user, everything you need to care about is in the `telemetry_exe` folder. Navigate to that directory.

There are two services: the **Live Console** and the **Parser and Plotter**

### Live Console
1. Either run the file `console_exe.py` with the Python Interpreter or issue the command `py -3 console_exe.py`
2. The default data source is the MQTT broker on the EC2 server. To change it to Test CSV mode or ESP32 mode, grab a data acquisition member

### Parser and Plotter
1. Get the raw data CSVs from the SD card on the vehicle
2. Place them in the `Raw_Data` folder in this directory
3. Either run the file `parser_exe.py` with the Python Interpreter or issue the command `py -3 parser_exe.py`
4. Wait for the process to finish (a success message from `parser_exe.py` followed by termination)
5. You may now retrieve the parsed data from the `Parsed_Data` folder and the .mat file `output.mat`

There is no need to delete the CSVs or the .mat file between use. However, the next time you run, be sure to delete the old CSVs from `Raw_Data` if desired. Otherwise, they will be parsed again.

_The next steps are optional - only if you want to plot the result_

6. Open `dataPlots.m` in MatLab
7. In MatLab, first load `output.mat` by double clicking it on the sidebar. Then click run on `dataPlots.m`
    - This script will not execute fully if there is not enough data, and it will stop on the first plot it is missing data for

## Developer's Guide
_Way too lazy to do right now, I'll get to it after comp hopefully_
