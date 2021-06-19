#include <SD.h>
#include <SPI.h>
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>

FlexCAN CAN(500000);
static CAN_message_t msg;
File logger;

/*
 * To correctly push this code onto the Teensy, you must push this code TWICE: first with the Teensy3Clock.set([time]) function below uncommented and the current epoch time inserted as the parameter [time].  This is to set the current time onto the RTC onboard.
 * Next, comment out the same line and push again.  This ensures that each time the Teensy "wakes", that same time parameter will not be set again. 
 */

void setup() {
  // put your setup code here, to run once:
    NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);                                   // Enables interrupts on the teensy for CAN messages
    attachInterruptVector(IRQ_CAN_MESSAGE, parse_can_message);          // Attaches parse_can_message() as ISR
    FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;                             // Allows "CAN message arrived" bit in flag register to throw interrupt

    Serial.begin(115200);
    CAN.begin();                                                        // Begin CAN
    SD.begin();                                                         // Begin Arduino SD API
    pinMode(10, OUTPUT);                                                // Initialize pin 10 as output; this is necessary for the SD Library
    logger = SD.open("sample.txt", FILE_WRITE);                         // Open file for writing.  Code revision necessary to record distinct sessions
    logger.println("time (uS), msg.id, data");                          // Print heading to the file.
    logger.flush();

    //Teensy3Clock.set(1536653782);                                       // set time (epoch) at powerup  (COMMENT OUT THIS LINE AND PUSH ONCE RTC HAS BEEN SET!!!!)
    setSyncProvider(getTeensy3Time);                                    // registers Teensy RTC as system time
}

void loop() {
    digitalClockDisplay();  
    delay(1000);
}

void parse_can_message() {                                              // ISR
  Serial.println("Received!!");
  CAN.read(msg);
  timestampWrite();
  logger.print(msg.id, HEX);
  logger.print(", ");
  for (int i=0; i<8; i++) {
    logger.print(msg.buf[i], HEX);
    logger.print(" ");
  }
  logger.println();
  logger.flush();                                                       // Ensure log is saved
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void timestampWrite() {
  logger.print(hour());
  writeDigits(minute());
  writeDigits(second());
  logger.print(" ");
  logger.print(day());
  logger.print(" ");
  logger.print(month());
  logger.print(" ");
  logger.print(year()); 
  logger.print("   ");
}

/*
 * What the frick is this function used for????????
 */
time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void writeDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  logger.print(":");
  if(digits < 10)
    logger.print('0');
  logger.print(digits);
}

