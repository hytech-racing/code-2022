#include <FlexCAN.h> // import teensy library

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
bool softwareFault = false; // true when software fault found
const int THERMISTORNOMINAL = 10000;
const int TEMPERATURENOMINAL = 25;
const int BCONSTANT = 3900;
const int SERIESRESISTOR = 10000;

// Values to check if IMD, BMS high
const int IMD_High = 50;
const int BMS_High = 50;

// timer
unsigned long AIRinitialTime; // use timer = millis() to get time, and compare in ms
unsigned long AIRcurTime;
unsigned long updateInitialTime = millis(); // timer for canUpdate function calls
unsigned long updateCurTime;

const int OKHS_PIN = 0;
const int BMS_OK_PIN = 1;
const int THERMISTOR_PIN = 4;

enum State { GLVinit=0, waitIMDBMS, waitDriver, AIRClose, fatalFault, drive }; // NOTE: change and update
State curState = GLVinit; // curState is current state

//FUNCTION PROTOTYPES
bool readValues();
bool checkFatalFault();
bool sendCanMessage(int, int, int);
bool sendCanUpdate();


// setup code
void setup() {
    Serial.begin(115200); // init serial for PC communication

    CAN.begin(); // init CAN system
    Serial.println("CAN system and serial communication initialized");
    curState = GLVinit; // curState is current state
    Serial.println("Current state is GLVinit");

}

// loop code
void loop() {
    readValues();
    Serial.print("TEMPERATURE: ");
    Serial.println(thermTemp);
    Serial.print("OKHS: ");
    Serial.println(OKHS);
    Serial.print("BMS: ");
    Serial.println(DISCHARGE_OK);
    Serial.println(thermValue);
    delay(200);
    
    //check CAN for a message for software shutdown
    if (!startupDone) {
        switch (curState) {
            case GLVinit:
                curState = waitIMDBMS; //going straight to waitIMD unti further notice
                break;
            case waitIMDBMS:
                if (softwareFault) {
                    curState = fatalFault;
                } else {
                    if (DISCHARGE_OK >= BMS_High) { // if BMS is high
                        if (OKHS >= IMD_High) { // if IMD is also high
                            curState = waitDriver; // both BMD and IMD are high, wait for start button press
                        }
                    }
                }
                break;
            case waitDriver:
                if (checkFatalFault()) {
                    curState = fatalFault;
                } else {
                    /*can message for start button press received*/
                    curState = AIRClose;
                }
                break;
            case AIRClose: // equivalent to VCCAIR in Google Doc state diagram
                AIRinitialTime = millis();
                AIRcurTime = millis();
                while(AIRcurTime <= AIRinitialTime + 500){
                    if (checkFatalFault()) {
                        curState = fatalFault;
                        break;
                    }
                    AIRcurTime = millis();
                }
                if (!(curState == fatalFault)) {
                    curState = drive;
                }
                break;
            case fatalFault:
                break;
            case drive:
                break;
            //send can message to throttle control
        }
    } else {
    }

    updateCurTime = millis();
    if((updateCurTime - updateInitialTime)%500 == 0){ //send updates every half second
      sendCanUpdate();
    }
}

bool readValues() {
    DISCHARGE_OK = analogRead(BMS_OK_PIN)/ 67.7;
    OKHS = analogRead(OKHS_PIN) / 67.7;
    thermValue = analogRead(THERMISTOR_PIN);
    //compute actual temperature with math
    float resistance = (5.0 * SERIESRESISTOR * 1023) / (3.3 * thermValue) - SERIESRESISTOR;
//    Serial.println(resistance);
    thermTemp = resistance / THERMISTORNOMINAL;
    thermTemp = log(thermTemp);
    thermTemp /= BCONSTANT;
    thermTemp += 1.0 / (TEMPERATURENOMINAL + 273.15);
    thermTemp = 1.0 / thermTemp;
    thermTemp -= 273.15;  
    return true;

    while(CAN.read(msg)) {
        if (msg.id == 0xBBBB) {
            softwareFault = true;
            
        }
    }
}

bool checkFatalFault() { // returns true if fatal fault found ()
    if (OKHS >= IMD_High && DISCHARGE_OK >= BMS_High && !softwareFault) {
        return false;
    } else {
        return true;
    }
}

bool sendCanMessage(int address, int msgLength, int data){
  
}

bool sendCanUpdate(){
    
}

