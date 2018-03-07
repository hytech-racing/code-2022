/*
 * screen.ino - 4D Systems Gen4-ULCD-43D Code
 * Brian Cai actually contributed something useful
 * on 2/6/2018
 */

 /*
  * There's a possibility that things may not show up because it's not 
  * the corresponding correct data type. It might also be too bright so 
  * the lights make it not readable fromt he background
  */
#include <genieArduino.h>
//#include <stdio.h>
//#include <stdint.h>
//#include <ctype.h>

#include <FlexCAN.h>
#include <HyTech17.h>

#ifndef TRUE
#define TRUE  (1==1)
#define FALSE  (!TRUE)
#endif

#define RESETLINE 2  // Change this for reset pin

//Display Objects
Genie genie;
int leddigits0 = 10; //battery temp high
int leddigits1 = 11; //battery temp low
int leddigits2 = 12; //Motor temp
int leddigits3 = 13; // GLV Voltage
int leddigits4 = 14; //Current Draw
int gauge0 = 51; // Battery Level
int coolgauge0 = 45; //Speed

//Message Reading 
FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
  // put your setup code here, to run once:
  Serial1.begin(9600);
  genie.Begin(Serial1);


  //genie.AttachEventHandler(myGenieEventHandler);//Processes Events

  //Reset display for that fresh in sync shit
  //change RESETLINE to 2 for D2
  pinMode(RESETLINE, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  digitalWrite(RESETLINE, 0);  // Reset the Display via D4
  delay(100);
  digitalWrite(RESETLINE, 1);  // unReset the Display via D4

  delay (3500); //let the display start up

  genie.WriteContrast(15); 
}

void loop() {
  // put your main code here, to run repeatedly:

  //read can bus
  while (CAN.read(msg)) {
      //initialize objects because c++ doesnt support switch statement object creation
      MC_current_information MCcurrentInfo;
      BMS_temperatures BMStemps;
      MC_motor_position_information motorposition;
      RCU_status rcu_status;
      MC_temperatures_3 mctemp3;
      
      switch(msg.id)
      {
        case ID_MC_CURRENT_INFORMATION: //current draw
              MCcurrentInfo = MC_current_information(msg.buf);
/*
        int16_t get_phase_a_current();
        int16_t get_phase_b_current();
        int16_t get_phase_c_current();
        int16_t get_dc_bus_current();
*/
              leddigits4 = MCcurrentInfo.get_phase_a_current();//Current Draw
              Serial1.print("Current Draw:");
              Serial1.println(leddigits4);
              break;
        case ID_BMS_TEMPERATURES: //low avg high battery temperature
              BMStemps = BMS_temperatures(msg.buf);
              leddigits0 = BMStemps.get_high_temperature(); //battery temp high
              leddigits1 = BMStemps.get_low_temperature(); //battery temp low
              Serial1.print("Battery Temperature High:");
              Serial1.println(leddigits0);
              Serial1.print("Battery Temperature Low:");
              Serial1.println(leddigits1);
              break;
        //case BATTERY LEVEL:
              //int gauge0 = 50; // Battery Level
              //printf();
              //break;
        case ID_MC_MOTOR_POSITION_INFORMATION: //speed (angular velocity)
              //Right Now its only the rotation
              motorposition = MC_motor_position_information(msg.buf);
              /*
        int16_t get_motor_angle();
        int16_t get_motor_speed();
        int16_t get_electrical_output_frequency();
        int16_t get_delta_resolver_filtered();
               */
              coolgauge0 = motorposition.get_motor_speed();
              Serial1.print("Motor Speed (Unrefined):");
              Serial1.println(coolgauge0);
              break;
        case ID_RCU_STATUS: //GLV Voltage
              rcu_status = RCU_status(msg.buf);
              leddigits3 = rcu_status.get_glv_battery_voltage();
              Serial1.print("GLV Voltage:");
              Serial1.println(leddigits3);
              break;
        case ID_MC_TEMPERATURES_3: //motor temperature              
              mctemp3 = MC_temperatures_3(msg.buf);
              leddigits2 = mctemp3.get_motor_temperature();
              Serial1.print("Motor Temperature:");
              Serial1.println(leddigits2);
              break;
      }
  }
  updateLCD();
  delay(50);

}

void updateLCD() {
  
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 0, leddigits0);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 1, leddigits1);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 2, leddigits2);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 3, leddigits3);
  genie.WriteObject(GENIE_OBJ_LED_DIGITS, 4, leddigits4);
  genie.WriteObject(GENIE_OBJ_GAUGE, 0, gauge0);
  genie.WriteObject(GENIE_OBJ_COOL_GAUGE, 0, coolgauge0);
}
