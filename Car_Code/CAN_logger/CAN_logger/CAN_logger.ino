#include <FlexCAN.h>
#include <HyTech17.h>
#include <kinetis_flexcan.h>
#include <Wire.h>
#include <SD.h>

FlexCAN CAN(500000);
static CAN_message_t msg;
File logger;

void setup() {
  // put your setup code here, to run once:
    NVIC_ENABLE_IRQ(IRQ_CAN_MESSAGE);                                   // Enables interrupts on the teensy for CAN messages
    attachInterruptVector(IRQ_CAN_MESSAGE, parse_can_message);          // Attaches parse_can_message() as ISR
    FLEXCAN0_IMASK1 = FLEXCAN_IMASK1_BUF5M;                             // Allows "CAN message arrived" bit in flag register to throw interrupt

    CAN.begin();                                                        // Begin CAN
    SD.begin();                                                         // Begin Arduino SD API
    pinMode(10, OUTPUT);                                                // Initialize pin 10 as output; this is necessary for the SD Library
    logger = SD.open("sample.txt", FILE_WRITE);                         // Open file for writing.  Code revision necessary to record distinct sessions
    logger.println("time (uS), msg.id, data");                          // Print heading to the file.
}

void loop() {
}

void parse_can_message() {                                              // ISR
  CAN.read(msg);
  logger.printf("%d, ", micros());
  logger.print(msg.id, HEX);
  logger.print(", ");
  for (int i=0; i<8; i++) {
    logger.print(msg.buf[0], HEX);
    logger.print(" ");
  }
  logger.println();
  logger.flush();                                                       // Ensure log is saved
}

