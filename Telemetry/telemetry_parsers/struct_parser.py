import os
import re
import pandas as pd
import scipy.io
import numpy as np
import dateutil.parser as dp
from os import listdir
from os.path import isfile, join
from datetime import datetime


def read_files():
    '''
    @brief: Reads parsed data files from Parsed_Data folder and returns a 
            list of file paths (as strings)
    @input: None
    '''
    try:
        path_name = 'Parsed_Data'

        file_path = []
        file_count = 0
        for root, dirs, files in os.walk(path_name, topdown=False):
            for name in files:
                if ".CSV" in name or ".csv" in name:
                    fp = os.path.join(root, name)
                    file_path.append(fp)
                    file_count += 1
    except:
        print('Process failed at step 1.')

    print('Step 1: found ' + str(file_count) + ' files in the Parsed_Data folder')
    return file_path

def create_dataframe(files = []):
    '''
    @brief: Reads parsed data file and creates a pandas dataframe.
            Each row is formatted to work with the Matlab parser. 
    @input: A list of files
    '''
    try:
        df_list = []
        for f in files:
            df = pd.read_csv(f)
            df_list.append(df)
    except:
        print('Process failed at step 2.')

    print('Step 2: created dataframes')

    return df_list

def get_time_elapsed(frames = []):
    skip = 0
    df_list = []
    try:
        for df in frames:
            skip += 1
            timestamps = [dp.isoparse(x) for x in df['time']]
            if(len(timestamps) != 0):
                start_time = min(timestamps)
                time_delta = [(x - start_time).total_seconds()
                        for x in timestamps]

                df['time_elapsed'] = pd.Series(time_delta)
                #print(time_delta[0])
                df_list.append(df)
            else:
                print(skip)
                continue
    except:
        print('Process failed at step 3.')

    print('Step 3: calculated elapsed time')
    return df_list

def create_output(frames = []):
    '''
    @brief: Formats dataframe data to work with the Matlab parser. 
    @input: A dataframe of the original CSV
    '''
    try:
        struct_list = []
        for df in frames:
            labels = df['label'].unique()
            files_list = []

            for label in labels:
                df_label = df[df['label'] == label]
                df_new = df_label[['time_elapsed', 'value']].copy()
                labels_list = df_new.values.tolist()
                files_list.append(labels_list)
             struct_list.append(files_list)
    except :
        print('Process failed at step 4.')

    print('Step 4: created struct')
    return struct_list

print("Step 0: starting...")
csv_files = read_files()
frames_list = create_dataframe(csv_files)
frames_list1 = get_time_elapsed(frames_list)
struct = create_output(frames_list1)
print("Process complete...")