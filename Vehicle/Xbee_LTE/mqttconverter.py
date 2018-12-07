import micropython
micropython.kbd_intr(-1)
from umqtt.robust import MQTTClient
from machine import Pin
import binascii
import time
import sys
MESSAGE_LENGTH = 15
MQTT_SERVER = 'hytech-telemetry.ryangallaway.me'     # MQTT broker hostname
MQTT_PORT = 1883                    # MQTT broker port (non-SSL)
connectedLight = Pin("D0", Pin.OUT, value=1)
def create_client():
    i = 0       # retry delay
    c = MQTTClient('hytech_car', MQTT_SERVER, MQTT_PORT)
    while True:
        _ = sys.stdin.buffer.read(-1) # pull stuff off the buffer so it doesn't crash
        try: 
            print('attempting to connect...')
            c.connect()
            print('connected to MQTT broker')
            connectedLight.value(0)
            c.publish('hytech_car/telemetry', 'Xbee Connected')
            return c
        except OSError as e:
            print('failed connection attempt #', i)
            i += 1
            c.delay(i)
c = create_client()
def read_uart():
    incomingFrame = b''
    zeros = 0
    while True:
        data = sys.stdin.buffer.read(-1)
        if data is not None:
            #print("got data", data)
            if data == b'\x00':
                #print('zero')
                zeros += 1
            else:
                zeros = 0
            if zeros == 3 or b'\x00\x00\x00' in data:
                #print('3 zeroes!')
                send_timestamp()
                zeros = 0
                data = data.replace(b'\x00\x00\x00', b'')
            incomingFrame += data
            #print(binascii.hexlify(incomingFrame)) # for verification purposes
            while b'\x00' in incomingFrame:
                end_index = incomingFrame.find(b'\x00')
                if end_index >= 16: # only send complete messages to save data
                    #print('First byte:', incomingFrame[0])
                    #print('End index:', end_index)
                    frame = incomingFrame[0:end_index + 1]
                    timestamp = str.encode(str(time.time()))        # epoch time
                    #print('sending message...', type(frame))
                    #print(timestamp, binascii.hexlify(frame))
                    send_mqtt(timestamp, frame)
                incomingFrame = incomingFrame[end_index + 1:]
def send_mqtt(timestamp, data):
    msg = timestamp + b',' + data
    #print('XBee says: sent message')
    #print(msg)
    c.publish('hytech_car/telemetry', msg)
def send_timestamp():
    timestamp = str.encode(str(time.time()))
    #print(timestamp) # send timestamp over serial
read_uart()