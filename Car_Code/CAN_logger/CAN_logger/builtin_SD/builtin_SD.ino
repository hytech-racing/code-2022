#include <SD.h>
#include <SPI.h>
#include <FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>

File logger;
FlexCAN CAN(500000);
static CAN_message_t msg;

void setup()
{
  
 // Open serial communications and wait for port to open:
  Serial.begin(9600); 
  CAN.begin();
  SD.begin(BUILTIN_SDCARD);
  logger = SD.open("test.txt", FILE_WRITE);

  //Teensy3Clock.set(xxxxxxx);
  setSyncProvider(getTeensy3Time);
}

void loop()
{
	if (CAN.available()) {
    CAN.read(msg);
    timestampWrite();
    logger.print(msg.id, HEX);
    logger.print(", ");
    for (int i=0; i<8; i++) {
      logger.print(msg.buf[i], HEX);
      logger.print(" ");
    }
    logger.println();
    logger.flush();
	}
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

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void writeDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  logger.print(":");
  if(digits < 10)
    logger.print('0');
  logger.print(digits);
}

