import binascii
from cobs import cobs
import os
import threading
import time
import paho.mqtt.client as mqtt

# MQTT_SERVER = 'localhost'
MQTT_SERVER = 'ec2-3-134-2-166.us-east-2.compute.amazonaws.com'

MQTT_PORT   = 1883                    # MQTT broker port (non-SSL)
MQTT_TOPIC  = 'hytech_car/telemetry'

TIMEZONE_OFFSET_MS = 0
EPOCH_OFFSET_MS = 946684800000 # Convert epoch-2000 time to epoch-1970 time

def unpack(frame):
	try:
		if len(frame) != 16:
			return -1
		decoded = cobs.decode(frame)
		return decoded if fletcher16(decoded[0:13]) == (decoded[14] << 8 | decoded[13]) else -1
	except Exception:
		return -1

def fletcher16(data):
	c0 = c1 = 0
	for i in data:
		c0 = (c0 + i) & 0xFF
		c1 = (c1 + c0) & 0xFF
	return (c1 << 8) | c0

hexstring = lambda x: binascii.hexlify(x).decode()

def mqtt_message(client, userdata, msg):
	comma_index = msg.payload.find(b',')
	data = unpack(msg.payload[comma_index + 1:-1])

	if data != -1:
		timestamp = int(msg.payload[0:comma_index].decode()) + EPOCH_OFFSET_MS + TIMEZONE_OFFSET_MS
		print('{0},{1},{2}'.format(timestamp, hexstring(data[4:5]), hexstring(data[5:13])))

def tz_message(client, userdata, msg):
	global TIMEZONE_OFFSET_MS
	message_time_sec = int(msg.payload.decode()) + EPOCH_OFFSET_MS // 1000
	timezone_offset_hr = (int(time.time()) - message_time_sec) // 3600
	TIMEZONE_OFFSET_MS = timezone_offset_hr * 3600 * 1000

def create_client(topic, handler):
	client = mqtt.Client()
	client.connect_async(MQTT_SERVER, MQTT_PORT, 60)
	client.on_connect = lambda *_: client.subscribe(topic)
	client.on_message = handler
	client.loop_start()
	return client

create_client('hytech_car/timezone_registration', tz_message)
create_client(MQTT_TOPIC, mqtt_message)
threading.Event().wait()