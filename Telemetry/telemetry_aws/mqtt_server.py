import time
import binascii
import threading
import paho.mqtt.client as mqtt
from db import DB, unpack, fletcher16

MQTT_SERVER = 'localhost'
# MQTT_SERVER = 'ec2-3-134-2-166.us-east-2.compute.amazonaws.com'
# MQTT_SERVER = 'hytech-telemetry.ryangallaway.me'     # MQTT broker hostname

MQTT_PORT   = 1883                    # MQTT broker port (non-SSL)
MQTT_TOPIC  = 'hytech_car/telemetry'

INFLUX_DB_NAME = 'hytech_telemetry'
TIMEZONE = 0

def mqtt_connect(client, userdata, flags, rc):
    client.subscribe(MQTT_TOPIC)
    print("Subscribed to", MQTT_TOPIC)
    client.publish(MQTT_TOPIC, "Python client connected")

def mqtt_message(client, userdata, msg):
    timestamp = msg.payload[0:msg.payload.find(b',')]
    frame = binascii.hexlify(msg.payload[msg.payload.find(b',') + 1:-1])
    data = unpack(''.join(chr(c) for c in frame))

    if data != -1:
        timestamp = int(timestamp.decode()) + 946684800000 + TIMEZONE #  LTE Module stuck in 1990. Fix if you can.
        influxDB.write(timestamp, data)

def tz_connect(client, userdata, flags, rc):
    client.subscribe('hytech_car/timezone_registration')

def tz_message(client, userdata, msg):
    global TIMEZONE
    TIMEZONE = (int(time.time()) - (int(msg.payload.decode()) + 946684800)) // 3600 * 3600000

def live():
    # Set up mqtt connection
    tz_client = mqtt.Client()
    tz_client.connect(MQTT_SERVER, MQTT_PORT, 60)
    tz_client.on_connect = tz_connect
    tz_client.on_message = tz_message
    tz_client.loop_start()

    mqtt_client = mqtt.Client()
    mqtt_client.connect(MQTT_SERVER, MQTT_PORT, 60)
    mqtt_client.on_connect = mqtt_connect
    mqtt_client.on_message = mqtt_message
    mqtt_client.loop_start()
    print("Loop start")

    threading.Event().wait()

influxDB = DB(INFLUX_DB_NAME, "ms")
live()
