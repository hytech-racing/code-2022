#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

/*
 * This is a basic test of the BNO055.
 * Its purpose is to test to make sure the device
 * is configured properly
 * and returning desired values through the simple
 * i2c connection.
 */

// uses the default i2c address (x28) and sensorID (-1)
// device must be called bno
Adafruit_BNO055 bno = Adafruit_BNO055();

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  /*
   * Initialize the sensor
   */
  if (!bno.begin())
  {
    Serial.println("No BNO055 device detected.");
    while (true); // infinite loop to prevent something from breaking
  }
  delay(1000);
  bno.setExtCrystalUse(true);
}

void loop()
{
  // put your main code here, to run repeatedly:
  sensors_event_t event;
  bno.getEvent(&event);

  /* Display the floating point data */
  Serial.print("X: ");
  Serial.print(event.orientation.x, 4);
  Serial.print("\tY: ");
  Serial.print(event.orientation.y, 4);
  Serial.print("\tZ: ");
  Serial.print(event.orientation.z, 4);
  Serial.println();

  delay(100);
}
