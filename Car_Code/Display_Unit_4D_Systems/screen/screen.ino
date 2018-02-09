#include <genieArduino.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE  (1==1)
#define FALSE  (!TRUE)
#endif

int leddigits0; //battery temp high
int leddigits1; //battery temp low
int leddigits2; //Motor temp
int leddigits3; // GLV Voltage
int leddigits4; //Current Draw
int gauge0; // Battery Level
int coolgauge0; //Speed

void setup() {
  // put your setup code here, to run once:
  genieSetup(115200);


}

void loop() {
  // put your main code here, to run repeatedly:
  
  while (1) {
    updateLCD();
    delay(50);
  }

}

void updateLCD() {
  genieWriteObj (GENIE_OBJ_LED_DIGITS, 0, leddigits0);
  genieWriteObj (GENIE_OBJ_LED_DIGITS, 1, leddigits1);
  genieWriteObj (GENIE_OBJ_LED_DIGITS, 2, leddigits2);
  genieWriteObj (GENIE_OBJ_LED_DIGITS, 3, leddigits3);
  genieWriteObj (GENIE_OBJ_LED_DIGITS, 4, leddigits4);
  genieWriteObj (GENIE_OBJ_GUAGE, 0, gauge0);
  genieWriteObj (GENIE_OBJ_COOL_GAUGE, 0, coolgauge0);
}
