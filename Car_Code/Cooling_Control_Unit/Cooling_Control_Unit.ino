#include <FlexCan.h> // import teensy library
#include <HyTech17.h>
#include <Metro.h>

#define DRIVE_STATE 8 //PLACEHOLDER, need to decide right value for drive state from TCU
#define FAN_IDLE 40 //PLACEHOLDER
#define FAN_PIN_1 A7 
#define FAN_PIN_2 A8 
#define FAN_PIN_3 A9
#define TEMP_THRESHOLD 40
//TODO: ports

FlexCAN CAN(500000);
static CAN_message_t msg;

//flow sensor variables
int pulses = 0;
float flowRate = 0.0;
int milli = 300 //time in milliseconds of each cycle to poll for
Metro pulseTimer = Metro(milli);

// State - use HyTech library
uint8_t state;

Metro timer = Metro(500); //adjust fan speeds every 500 millis
bool started =  false;
int fanSpeed_1 = FAN_IDLE; 
int fanSpeed_2 = FAN_IDLE;
int fanSpeed_3 = FAN_IDLE;
int pumpPin = 12; //arbitrary
// setup code
void setup() {
    Serial.begin(115200); // init serial

    //Pins for flow sensor
    pinMode(D5, INPUT);
    pinMode(D6, INPUT);
    pinMode(D7, INPUT);
    pinMode(D8, INPUT);
    pinMode(D9, INPUT);
    pinMode(D10, INPUT);
    pinMode(D11, INPUT);
    pinMode(D12, INPUT);
    pinMode(D13, INPUT);
    pinMode(A0, OUTPUT);
    pinMode(A1, INPUT);

    //fan output pins
    pinMode(FAN_PIN_1, OUTPUT);
    pinMode(FAN_PIN_2, OUTPUT);
    pinMode(FAN_PIN_3, OUTPUT);

    CAN.begin();
    Serial.println("CAN system and serial communication initialized");
}

// loop code
void loop() {
    //check the flow sensor if timer is up
    if(pulseTimer.check()) { 
        int q1 = digitalRead(D5);
        int q2 = digitalRead(D6);
        int q3 = digitalRead(D7);
        int q4 = digitalRead(D8);
        int q5 = digitalRead(D9);
        int q6 = digitalRead(D10);
        int q7 = digitalRead(D11);
        int q8 = digitalRead(D12);
        int q9 = digitalRead(D13);
        
        // if a plastic sensor use the following calculation
        // Sensor Frequency (Hz) = 7.5 * Q (Liters/min)
        // Liters = Q * time elapsed (fseconds) / 60 (seconds/minute)
        // Liters = (Frequency (Pulses/second) / 7.5) * time elapsed (seconds) / 60
        // Liters = Pulses / (7.5 * 60)
        pulses = q1 * (1 << 0) + q2 * (1 << 1) + q3 * (1 << 2) + q4 * (1 << 3) + q5 * (1<<4) 
            + q6 * (1<<5) + q7 * (1<<6) + q8 * (1<<7) + q9 * (1<<8);
        Serial.print("pulses: ");
        Serial.println(pulses);
        float freq = pulses/((float)milli/1000);
        flowRate = freq/7.5;
        Serial.print("flow rate: ");
        Serial.println(flowRate);
        digitalWrite(6, HIGH);
        digitalWrite(6, LOW);
        pulseTimer.reset();
    }
    if (timer.check()) {
        while (CAN.read(msg)) {
            if (!started && msg.id == 0xD1 && msg.buf[1] == DRIVE_STATE) {
                started = true;
                //initial start up of car, so start up all fans and pump to some initial speed
                analogWrite(FAN_PIN_1, fanSpeed_1);
                analogWrite(FAN_PIN_2, fanSpeed_2);
                analogWrite(FAN_PIN_3, fanSpeed_3);
            }
            if (started) {
                // TODO Read AVG_TEMP message from components and utilize PWM to change
                // fan speed based on temps
                // Code not yet written on BMS to send temperatures
    
                // TODO Handle CAN messages from other components
                // Placeholder code
                // Will update when I figure out ports for fans, how to increase
                // fan speed, which fans to increase the speed of, and how to adjust
                // fan speed based on temperature
                if (msg.id == 0x51 || msg.id == FAN_ID_2 || msg.id == FAN_ID_3 { //TODO: PLACEHOLDER IDs
                    if (msg.buf[0] > TEMP_THRESHOLD) {
                        // Increase fan speed with PWM
                        if (msg.id==0x51) {
                            fanSpeed_1 +=1;
                            analogWrite(FAN_PIN_1, fanSpeed_1);
                        } else if (msg.id==FAN_ID_2) {
                            fanSpeed_2 +=1;
                            analogWrite(FAN_PIN_2, fanSpeed_2);
                        } else if (msg.id==FAN_ID_3) {
                            fanSpeed_3 +=1;
                            analogWrite(FAN_PIN_2, fanSpeed_3);
                        }
                    } else if (msg.buf[0] < TEMP_THRESHOLD){
                        // Decrease fan speed with PWM
                        if (msg.id==0x51 && fanSpeed_1 > FAN_IDLE) {
                            fanSpeed_1 -= 1;
                            analogWrite(FAN_PIN_1, fanSpeed_1);
                        } else if (msg.id==FAN_ID_2 && fanSpeed_2 > FAN_IDLE) {
                            fanSpeed_2 -= 1;
                            analogWrite(FAN_PIN_2, fanSpeed_2);
                        } else if (msg.id==FAN_ID_3 && fanSpeed_3 > FAN_IDLE) {
                            fanSpeed_3 -= 1;
                            analogWrite(FAN_PIN_3, fanSpeed_3);
                        }
                    }
                    sendCanUpdate();
                }
            }
        }
    }
}

int sendCanUpdate() {
    //TODO write CAN update code
}
