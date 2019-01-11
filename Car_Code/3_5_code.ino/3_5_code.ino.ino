#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <ADC_SPI.h>
#include <HyTech_FlexCAN.h>
#include <HyTech_CAN.h>
#include <kinetis_flexcan.h>
#include <Wire.h>
#include <SD.h>
#include <TimeLib.h>
#include <Metro.h>

FlexCAN CAN(500000);
static CAN_message_t msg;
File logger;
FCU_accelerometer_values fcu_accelerometer_values;
ADC_SPI ADC(10);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

Metro timer_accelerometer = Metro(100);
Metro timer_current_cooling = Metro(500);
Metro timer_current_ecu = Metro(500);

/*
 * To correctly push this code onto the Teensy, you must push this code TWICE: first with the Teensy3Clock.set([time]) function below uncommented and the current epoch time inserted as the parameter [time].  This is to set the current time onto the RTC onboard.
 * Next, comment out the same line and push again.  This ensures that each time the Teensy "wakes", that same time parameter will not be set again. 
 */

void setup() {
  // put your setup code here, to run once:
    NVIC_ENABLE_IRQ(IRQ_CAN0_MESSAGE);                                   // Enables interrupts on the teensy for CAN messages
    attachInterruptVector(IRQ_CAN0_MESSAGE, parse_can_message);          // Attaches parse_can_message() as ISR
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

    setupAccelerometer();

    pinMode(A12, INPUT);
    pinMode(A13, INPUT);
}

void loop() {
    digitalClockDisplay();  
    delay(1000);
    
    if (timer_accelerometer.check()) {
        processAccelerometer(); 
    }
    if (timer_current_ecu.check()) {
        process_current(analogRead(A13)); 
    }
    if (timer_current_cooling.check()) {
        process_current(analogRead(A12));
    }
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

void setupAccelerometer() {
      
  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Sensor not detected!!!!!");
  }
  else {
    accel.setRange(ADXL345_RANGE_4_G);
  }
}

void processAccelerometer() {
  /* Get a new sensor event */ 
  sensors_event_t event; 
  accel.getEvent(&event);
  
  /* Read accelerometer values into accelerometer struct */
  fcu_accelerometer_values.set_values(event.acceleration.x, event.acceleration.y, event.acceleration.z);

  /* Send msg over CAN */
  noInterrupts();
  fcu_accelerometer_values.write(msg.buf);
  msg.id = ID_FCU_ACCELEROMETER;
  msg.len = sizeof(CAN_message_fcu_accelerometer_values_t);
  CAN.write(msg);
  interrupts();
  
  Serial.print("\n\nACCELEROMETER DATA\n\n");
  Serial.print(event.acceleration.x); Serial.print(", ");
  Serial.print(event.acceleration.y); Serial.print(", ");
  Serial.print(event.acceleration.z); Serial.println("\n\n");
}

void process_current(int input) {
  double current = (((double)input)/1023*5)/0.185;
  
}

