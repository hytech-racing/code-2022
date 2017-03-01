#include <FlexCAN.h> // import teensy library
#include <Metro.h>
#include <HyTech17.h>

// TODO: Convert to HyTech library
// TODO: Create debug flag and debug mode (serial printing status values)
#define debugFlag true

//TODO make sure faults are in CAN library
#define BMS_FAULT (1<<0)
#define IMD_FAULT (1<<1)
#define BSPD_FAULT (1<<2)

#define THERMISTORNOMINAL 10000
#define TEMPERATURENOMINAL 25
#define BCONSTANT 3900
#define SERIESRESISTOR 10000

// Values to check if IMD, BMS high
#define IMD_High 9
#define BMS_High 3

// Pins
#define OKHS_PIN 0
#define BMS_OK_PIN 1
#define THERMISTOR_PIN 4
#define MC_SWITCH_SSR_PIN 6
#define BMS_LATCH_SSR_PIN 11
#define IMD_LATCH_SSR_PIN 10
#define BRAKE_LIGHT_PIN 13

FlexCAN CAN(500000);
static CAN_message_t msg;
float OKHS = 0; // voltage after calculation
float DISCHARGE_OK = 0; // voltage after calculation
float GLVbattery = 0; // voltage after calculation
float shutdownCircuit = 0; // voltage after calculation
bool OKHSfault = false; // fault status of OKHS, fault if true
bool DISCHARGE_OKfault = false; // fault status of DISCHARGE_OK, fault if true
bool startPressed = false; // true if start button is pressed
float thermTemp = 0.0; // temperature of onboard thermistor (after calculation)
int thermValue = 0; //raw value from thermistor
bool startupDone = false; // true when reached drive state
uint8_t faultId = 0;

// timer
Metro CANUpdateTimer = Metro(500); // Used for how often to send state
Metro updateTimer = Metro(500); // Read in values from inputs
Metro AIRtimer = Metro(500);

uint8_t state; // state from HyTech Library

//FUNCTION PROTOTYPES
bool readValues();
bool checkFatalFault();
int sendCanUpdate();

// setup code
void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");

    // Set up SSR output pins
    pinMode(BMS_LATCH_SSR_PIN, OUTPUT);
    pinMode(IMD_LATCH_SSR_PIN, OUTPUT);
    pinMode(BRAKE_LIGHT_PIN, OUTPUT);
    pinMode(MC_SWITCH_SSR_PIN, OUTPUT);

    startPressed = 0;
}

void loop() {
    checkFatalFault();

    /*
     * Handle incoming CAN messages
     */
    while (CAN.read(msg)) {
      if (msg.id > 0xBF) {
        Serial.print(msg.id, HEX);
        Serial.print(": ");
        for (unsigned int i = 0; i < msg.len; i++) {
          Serial.print(msg.buf[i], HEX);
          Serial.print(" ");
        }
        Serial.println();
      }

      // Scanning CAN for dashboard state message (start button)
      // Change values based on what message actually is
      if (state == PCU_STATE_WAITING_DRIVER && msg.id == ID_PCU_STATUS) {
        Serial.println(msg.buf[0], BIN);
        if (msg.buf[0] & 0x10) {
          startPressed = 1;
        }
      }
    }

    switch (state) {
        case GLVinit:
            state = PCU_STATE_WAITING_BMS_IMD; //going straight to waitIMD until further notice
            break;
        case PCU_STATE_WAITING_BMS_IMD:
            if (debugFlag)
                Serial.println("Waiting for IMD/BMS OK...");
            if (DISCHARGE_OK >= BMS_High) { // if BMS is high
                if (OKHS >= IMD_High) { // if IMD is also high
                    state = PCU_STATE_WAITING_DRIVER; // both BMD and IMD are high, wait for start button press

                    // Once adapted to library, we can change MC enabling to depend on throttle control
                    // status message
                    digitalWrite(MC_SWITCH_SSR_PIN, HIGH) // enable MC
                }
            }
            break;
        case PCU_STATE_WAITING_DRIVER:
            if (debugFlag)
                Serial.println("Waiting for start button...");
            /*can message for start button press received*/

            if (startPressed) {
                AIRtimer.reset();
                if (debugFlag)
                    Serial.println("Latching...");
                digitalWrite(BMS_LATCH_SSR_PIN, HIGH);   // close latch A SSR (Software Switch)
                digitalWrite(IMD_LATCH_SSR_PIN, HIGH);   // close latch B SSR
                state = PCU_STATE_LATCHING;
            }
            break;
        case PCU_STATE_LATCHING: // equivalent to VCCAIR in Google Doc state diagram
            if(AIRtimer.check()){
                if (debugFlag)
                    Serial.println("Completed latching");
                digitalWrite(BMS_LATCH_SSR_PIN, LOW);  // Open latch SSR
                digitalWrite(IMD_LATCH_SSR_PIN, LOW);  // open latch B SSR
                state = PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED;
            }
            startPressed = false;
            break;
        case PCU_STATE_FATAL_FAULT:
            Serial.println("FAULTED");
            switch (faultId) {
                case 1 :
                    Serial.println("BMS Fault Detected by PCU");
                    break;
                case 2:
                    Serial.println("IMD Fault Detected by PCU");
                    break;
                case 3:
                    Serial.println("BSPD Fault detected by PCU");
                    break;
            }
            break;
        case PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED:
            if (debugFlag)
                Serial.println("Drive state");
            break;
    }

    if(CANUpdateTimer.check()){
      sendCanUpdate();
      CANUpdateTimer.reset();
    }
    if(updateTimer.check()){
        readValues();
        updateTimer.reset();
    }
}

bool readValues() {
    DISCHARGE_OK = analogRead(BMS_OK_PIN)/ 67.7;
    OKHS = analogRead(OKHS_PIN) / 67.7;
    thermValue = analogRead(THERMISTOR_PIN);
    //compute actual temperature with math
    float resistance = (5.0 * SERIESRESISTOR * 1023) / (3.3 * thermValue) - SERIESRESISTOR;
    if (debugFlag)
        Serial.println("Resistance: " + resistance);
    thermTemp = resistance / THERMISTORNOMINAL;
    thermTemp = log(thermTemp);
    thermTemp /= BCONSTANT;
    thermTemp += 1.0 / (TEMPERATURENOMINAL + 273.15);
    thermTemp = 1.0 / thermTemp;
    thermTemp -= 273.15;
    return true;

}

bool checkFatalFault() { // returns true if fatal fault found
    CAN_message_t faultMsg;
    faultMsg.buf[0] = 0;
    if (state == PCU_STATE_WAITING_DRIVER || state == PCU_STATE_LATCHING || state == drive) {
      if (OKHS < IMD_High) {
          faultMsg.buf[0] = faultMsg.buf[0] | IMD_FAULT;
      }
      if (DISCHARGE_OK < BMS_High) {
          faultMsg.buf[0] = faultMsg.buf[0] | BMS_FAULT;
      }
    }

    while (CAN.read(msg)) {
        if (msg.id == 0x0001) {
            faultMsg.buf[0] = faultMsg.buf[0] | BSPD_FAULT;
        }
    }


    if (faultMsg.buf[0] != 0) {
        Serial.println("FATAL FAULT OCCURRED");
        Serial.print("FAULT ID: ");
        Serial.println(faultMsg.buf[0], BIN);
        state = PCU_STATE_FATAL_FAULT;
        faultId = faultMsg.buf[0];
        faultMsg.id = 0x002;
        faultMsg.len = 1;
        CAN.write(faultMsg);
        return true;
    } else {
        return false;
    }
}

int sendCanUpdate(){

    //prepare to send the voltages as shorts in the CAN message
    short shortDischargeOk = (short) (DISCHARGE_OK * 10);
    short shortOKHS = (short) (OKHS * 10);
    short shortGLV = (short) (GLVbattery * 10);
    short shortShutdown = (short) (shutdownCircuit * 10);

    //send the message
    msg.id = 0x50;
    msg.len = 8;
    memcpy(&msg.buf[0], &shortDischargeOk, sizeof(short));
    memcpy(&msg.buf[2], &shortOKHS, sizeof(short));
    memcpy(&msg.buf[4], &shortGLV, sizeof(short));
    memcpy(&msg.buf[6], &shortShutdown, sizeof(short));

    int temp1 = CAN.write(msg);

    bool okhsCheck = OKHS >= IMD_High;
    bool dischargeCheck = DISCHARGE_OK >= BMS_High;
    short shortTemp = (short) thermTemp * 100;

    msg.id = 0x51;
    msg.len = 5;
    memcpy(&msg.buf[0], &shortTemp, sizeof(short));
    memcpy(&msg.buf[2], &okhsCheck, sizeof(bool));
    memcpy(&msg.buf[3], &dischargeCheck, sizeof(bool));
    memcpy(&msg.buf[4], &PCU_STATE_FATAL_FAULT, sizeof(byte));

    int temp2 = CAN.write(msg);

    return temp1 + temp2;
}
