/*
 * testscreen.ino - Test 4D Systems Gen4-ULCD-43D Code
 * Brian Cai
 * 3/27/2018
 */
#include <HyTech_FlexCAN.h>
#include <HyTech17.h>
#include <Metro.h>

/*
 */
FlexCAN CAN(500000);
static CAN_message_t msg;
MC_current_information MCcurrentInfo;
BMS_temperatures BMStemps;
MC_motor_position_information motorposition;
RCU_status rcu_status;
MC_temperatures_3 mctemp3;
BMS_voltages bmsvoltages;

///*
// * Timers
// */
//Metro timer_debug_current_draw = Metro(1000);
//Metro timer_debug_bms_temperature_high = Metro(1000);
//Metro timer_debug_bms_temperature_low = Metro(1000);
//Metro timer_debug_motor_rotation = Metro(1000);
//Metro timer_debug_glv_voltage = Metro(2000);
//Metro timer_debug_motor_temperature = Metro(2000);
//Metro timer_debug_total_voltage = Metro(2000);

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    CAN.begin();
    BMStemps.set_high_temperature(40); //battery temp high
    BMStemps.set_low_temperature(30); //battery temp low
    //motorposition.set_motor_speed(150);
    rcu_status.set_glv_battery_voltage(11); 
    //mctemp3.set_motor_temperature(36);
    bmsvoltages.set_total(19); //total voltage
}

void loop() {
    msg.id = ID_MC_CURRENT_INFORMATION;
    msg.len = sizeof(CAN_message_mc_current_information_t);  
    //no set function
    //McCurrentInfo.write(msg.buf);
    int twenty = 20;
    memcpy(&(twenty), &(msg.buf[0]), sizeof(int16_t));

    CAN.write(msg);
    Serial.println("Current Draw:");
    for (int i = 0; i<8; i++) {
      Serial.print(msg.buf[i], HEX);          
      Serial.print(" ");          
    }
    Serial.println();

    msg.id = ID_BMS_TEMPERATURES;
    msg.len = sizeof(CAN_message_bms_temperatures_t);  
    BMStemps.write(msg.buf);
    CAN.write(msg);
    Serial.println("BMStemps:");
    for (int i = 0; i<8; i++) {
      Serial.print(msg.buf[i], HEX);          
      Serial.print(" ");          
    }
    Serial.println();

    msg.id = ID_BMS_VOLTAGES;
    msg.len = sizeof(CAN_message_bms_voltages_t);  
    bmsvoltages.write(msg.buf);
    CAN.write(msg);
    Serial.println("BMS Voltages:");
    for (int i = 0; i<8; i++) {
      Serial.print(msg.buf[i], HEX);          
      Serial.print(" ");          
    }
    Serial.println();

    msg.id = ID_MC_MOTOR_POSITION_INFORMATION;
    msg.len = sizeof(CAN_message_mc_motor_position_information_t);  
    //no set method
    //motorposition.write(msg.buf);
    int onefifty = 150;
    memcpy(&(onefifty), &(msg.buf[2]), sizeof(int16_t));
    CAN.write(msg);
    Serial.println("Motor Position:");
    for (int i = 0; i<8; i++) {
      Serial.print(msg.buf[i], HEX);          
      Serial.print(" ");          
    }
    Serial.println();


    msg.id = ID_RCU_STATUS;
    msg.len = sizeof(CAN_msg_rcu_status);  
    rcu_status.write(msg.buf);
    CAN.write(msg);
    Serial.println("RCU Status/GLV Voltage:");
    for (int i = 0; i<8; i++) {
      Serial.print(msg.buf[i], HEX);          
      Serial.print(" ");          
    }
    Serial.println();

    msg.id = ID_MC_TEMPERATURES_3;
    msg.len = sizeof(CAN_message_mc_temperatures_3_t);  
    //mctemp3.write(msg.buf);
    int thirtysix = 36;
    memcpy(&(thirtysix), &(msg.buf[4]), sizeof(int16_t));
    CAN.write(msg);
    Serial.println("Motor Temperature:");
    for (int i = 0; i<8; i++) {
      Serial.print(msg.buf[i], HEX);          
      Serial.print(" ");          
    }
    Serial.println();
  
}

