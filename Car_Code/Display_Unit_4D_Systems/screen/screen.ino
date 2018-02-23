#include <genieArduino.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE  (1==1)
#define FALSE  (!TRUE)
#endif

#define RESETLINE 2  // Change this if you are not using an Arduino Adaptor Shield Version 2 (see code below)

Genie genie;
int leddigits0 = 10; //battery temp high
int leddigits1 = 11; //battery temp low
int leddigits2 = 12; //Motor temp
int leddigits3 = 13; // GLV Voltage
int leddigits4 = 14; //Current Draw
int gauge0 = 50; // Battery Level
int coolgauge0 = 69; //Speed

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
  
  while (1) {
    updateLCD();
    delay(50);
  }

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
