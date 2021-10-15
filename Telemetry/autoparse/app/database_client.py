#!/usr/bin/python3

import csv
from influxdb import InfluxDBClient
import requests
import sys
import threading
import time

INFLUX_HOST = 'localhost'
INFLUX_PORT = 8086

class DatabaseClient:
	def __init__(self, INFLUX_DB_NAME):
		self.influx_client = None

		while True:
			print('Attempting to connect to database at {}:{}'.format(INFLUX_HOST, INFLUX_PORT))
			try:
				self.influx_client = InfluxDBClient(host=INFLUX_HOST, port=INFLUX_PORT)
				db_exists = False
				for db in self.influx_client.get_list_database():
					if db['name'] == INFLUX_DB_NAME:
						db_exists = True
						break
				if not db_exists:
					requests.get('http://{}:{}/query?q=CREATE+DATABASE+"{}"'.format(INFLUX_HOST, INFLUX_PORT, INFLUX_DB_NAME))
				self.influx_client.switch_database(INFLUX_DB_NAME)
				break
			except Exception:
				print("Influx connection refused. Trying again in ten seconds.")
				time.sleep(10)

		print("Connected using database {}".format(INFLUX_DB_NAME))
		self.json_body = []
		self.writing = True

	def write(self, data):
		# print("Writing document: ")
		# print(data)
		
		self.json_body.append(data)

	def buffered_write(self):
		while self.writing:
			json = self.json_body
			self.json_body = []

			if not json:
				time.sleep(5)
				continue

			# print("Writing set: ")
			# print(json)

			try:
				self.influx_client.write_points(points=json)
			except Exception as e:
				print("Operation failed. Printing error:")
				print(e)

	def shutdown(self):
		self.writing = False

def get_value(val):
	try:
		return float(val)
	except Exception:
		return val

if len(sys.argv) < 2:
	print("Usage: {} [DATABASE NAME]".format(sys.argv[0]))
	sys.exit(0)

telem_client = DatabaseClient(sys.argv[1])

reader = csv.reader(sys.stdin)
HEADER = next(reader)

TIME_COL = HEADER.index('time')
ID_COL = HEADER.index('id')
MSG_COL = HEADER.index('message')
LABEL_COL = HEADER.index('label')
VALUE_COL = HEADER.index('value')
UNIT_COL = HEADER.index('unit')

writer = threading.Thread(target=telem_client.buffered_write).start()

for record in reader:
	telem_client.write({
		'measurement': record[LABEL_COL],
		'time': record[TIME_COL],
		'fields': { 'value': get_value(record[VALUE_COL]) },
		'tags': {
			'message': record[MSG_COL],
			'unit': record[UNIT_COL] if len(record) > UNIT_COL else ''
		}
	})

telem_client.shutdown()
writer.join()
