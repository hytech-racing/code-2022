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
### Maintenance
1. Every time you add a pip package to a Python script that is not inherent to Python's native download, add it to `requirements.txt` if it does not exist. You can reference the current document, or https://note.nkmk.me/en/python-pip-install-requirements/
2. In addition to this repo, make sure the XBee and Live Console are connected to the same EC2 instance and that the EC2 instance is running. You can check the status of instances here: https://us-east-2.console.aws.amazon.com/ec2/v2/home?region=us-east-2#Instances:instanceState=running (login required)
3. To allow the XBee to wirelessly transmit, make sure the HyTech AT&T LTE account is activated and running. Ask the Treasurer for more info.
4. We are not really using the `telemetry_aws` folder other than the console importing a function from `db.py`. No need to modify anything in that folder.

### Adding/Modifying a CAN message
Various components of the Data Acquisition subsystem needs to be changed when a CAN message structure is modified or a new CAN message is added. These include:
- Parser: mandatory changes. There is a DEBUG flag for optional debugging print statements as you add code.
    - `multipliers.py`: may need changes accordingly if scaling is changed
- Live Console: additionals may need to be made; if no CAN message is added, then no changes are necessary unless a CAN label from the message has its name changed
- Plotter: may need change depending on the message

What should not need changing:
- `telemetry_aws` folder
- The `Parsed_Data` and `Raw_Data` sub-folders within `telemetry_exe`
- `parser_exe.py` (the parser launcher, does not include any actual parsing)

#### Steps
1. In `parser_api.py`:
    - In the `parse_message(raw_id, raw_message)` function, add the corresponding CAN ID and parsing function you will create later to the big if-statements. For example, `if raw_id == "D5": return parse_ID_BMS_ONBOARD_TEMPERATURES(raw_message)`
    - Create a new custom parsing function under the section denoted "Custom Parsing Functions Begin". The syntax is usually "parse_ID_CAN_MESSAGE_NAME" and takes in one parameter `raw_message`, e.g. `def parse_ID_MC_VOLTAGE_INFORMATION(raw_message):`
    - The custom parsing function is expected to return a ragged-2D array. The data type placeholders looks something like this `[string, string arr[num_labels], double arr[num_labels], string arr[num_labels]]`. For example, a CAN message parsed into 4 labels will look like:
    ["some_string",
    ["some_string", "some_string", "some_string", "some_string"],
    [some_double, some_double, some_double, some_double],
    ["some_string", "some_string", "some_string", "some_string"]]
    - The parser will later dimension-check the return array to make sure the last three sub-arrays are of the same size; else, it will discard the entire output.
    - Inside the custom parsing function, you will need to extract the individual bytes from the `raw_message` input string and perform arithmic/bitwise operations as needed. There is no good one-method-fits-all as this step is highly message dependent. Likely, you will need to utilize the `hex_to_decimal` function in some manner as Python's endianness and our CAN system's are not the same. See existing messages for inspiration.
    - If you just want to prototype but have no wish to implement the function parsing, you may simple return the string "UNPARSEABLE" for the custom parsing function.
2. In `multiplers.py`: Add scaling constants for the message if needed. They will be needed in the custom parsing functon in `parser_api.py` when performing arithmetic operations.
3. In `console_exe.py`:
    - `DICT` is a nested dictionary containing console subsection headers followed by display labels. The display labels are the corresponding CAN labels, but in all UPPERCASE. If you add a message to an existing subsection, all you need to do for the message to show up properly in the console is add the label under the corresponding header.
    - If you need to create a new header, you can do so in `DICT`. However, keep in mind you will need to allow PySimpleGUI to format and attach its updates to events. This is done in `def main()` and the complexity of which lies beyond the purpose of this README. You may reference PySimpleGUI resources on its official website: https://pysimplegui.readthedocs.io/en/latest/cookbook/
    - No need to modify the callback threads, e.g. `mqtt_connection_thread`
4. In `dataPlots.m`, you may need to make changes if a label name changes or if you want to create a new plot. Reference the current structure.
