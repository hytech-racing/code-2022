![HyTech Logo](https://hytechracing.gatech.edu/images/hytech_logo_small.png)
# HyTech Custom Parsers
## Intro
Welcome to the folder containing all the new updated parsers y'all ever need for HyTech parsing!
Here's a description to each file located in this folder:
- `parser.py`: "The main file"
    - Parses a folder named "Raw_Data" with raw CSVs from the SD card and outputs the parsed CSVs to a folder named "Parsed_Folder"
    - Will create a Parsed_Folder directory if it doesn't exist, but a Raw_Data folder is necessary
    - Then creates a .mat struct based on the Parsed_Folder for plotting and for our lovely MechEs
- `multipliers.py`: Contains a list of scaling constants for different message values used by parser.py
- `dataPlots.m`: Takes the .mat created by mat_parser.py and creates multiple plots of useful values.
- `dataExtracts.m`: Don't use, it's an alternative/deprecated MatLab-based equivalent of struct_parser.py but much slower

## Requirements
The entire system is Python and MatLab based.
- Python 3 is required, and may be installed here: https://www.python.org/downloads/. Make sure it is added to PATH.
- For MatLab, you will need an education/work license to get it for free. Follow GT's instructions here: https://www.matlab.gatech.edu/.

The Python parts require some libraries to run. After downloading Python, issue the following pip commands:
```
pip install DateTime
pip install numpy
pip install scipy
pip install pandas
pip install python-dateutil
```

## User Guide
1. Get the raw data CSVs from the SD card on the vehicle.
2. Place them in a folder called Raw_Data.
3. Copy `parser.py`, `multipliers.py`, and `dataPlots.m` into the directory containing the Raw_Data folder
4. Run `parser.py` by either using the Python Interpreter or by going into the directory and issuing: `py parser.py`
5. Wait for a success message from `parser.py`
6. Open `dataPlots.m` in MatLab
7. In MatLab, first load `output.mat` by double clicking it on the sidebar. Then click run on `dataPlots.m`.
