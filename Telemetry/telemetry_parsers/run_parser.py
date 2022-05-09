from parser_functions import *

########################################################################
# Entry Point to Framework
########################################################################
print("Welcome to HyTech 2022 Parsing Framework")
print("The process will be of two parts: CSV to CSV parsing, and then CSV to MAT parsing.")
print("The entire process will take about 5 mins for a test session's worth of data.")
print("----------------------------------------------------------------------------------")
print("Beginning CSV to CSV parsing...")
parse_folder()
print("Finished CSV to CSV parsing.")
print("----------------------------------------------------------------------------------")
print("Beginning CSV to MAT parsing...")
create_mat()
print("Finished CSV to MAT parsing.")
print("----------------------------------------------------------------------------------")
print("SUCCESS: Parsing Complete.")