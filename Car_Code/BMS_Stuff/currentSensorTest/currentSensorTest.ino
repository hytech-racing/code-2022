/*
 * 1. Find current sensor data sheet. save it.
 * 2. Hook up power and ground to second power source (from minifit jr. pin things) with alligator clips.
 * 3. Hook up current pin from minifit jr. pin thing to wire to arduino pin. Analog pin 2 for example.
 * 4. Write code.
 * 5. Wire resistor and current sensor in series with power source 1.
 * 6. Turn on sensor power, source 2
 * 7. Start code
 * 8. Verify 2.5 volts, 0 current.
 */

#define currentPin A2

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
    int reading = analogRead(currentPin);
    double vReading = (reading / 1023.0) * 5;
    // (reading - 2.5) / (2.4) * 300;
    double current = (vReading - 2.5) / 2 * 300;
    Serial.print("current: "); Serial.println(current);
    Serial.print("voltage: "); Serial.println(vReading);
    Serial.println();
}


