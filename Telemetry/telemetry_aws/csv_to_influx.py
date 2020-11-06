import sys
import binascii
import csv
from db import DB

if (len(sys.argv) < 3):
    print("Usage: db.py [DB_NAME] [FILEPATH]")
    exit()

influxDB = DB(sys.argv[1], "s")
with open(sys.argv[2], 'r') as file:
    reader = csv.reader(file)
    next(reader)
    for row in reader:
        influxDB.write(int(row[0]), binascii.unhexlify((row[1] + "00000000" + row[3]).ljust(26, '0').encode()))
