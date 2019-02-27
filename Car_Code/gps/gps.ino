#include <Adafruit_GPS.h>
#include <Metro.h>

Adafruit_GPS GPS(&Serial1);
Metro gpsTimer = Metro(1);

void setup() {
  // put your setup code here, to run once:
  GPS.begin(9600);    // this baud rate is necessary!!!
  Serial.begin(115200);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);     // specify data to be received (minimum + fix)
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);        // set update rate (1Hz)
  GPS.sendCommand(PGCMD_ANTENNA);                   // report data about antenna
}

void loop() {
  // put your main code here, to run repeatedly:
  if (gpsTimer.check()) {
    GPS.read();
  }
  
  if (GPS.newNMEAreceived()) {
    GPS.parse(GPS.lastNMEA());
    /*
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Location (in degrees, works with Google Maps): ");
      Serial.print(GPS.latitudeDegrees, 4);
      Serial.print(", "); 
      Serial.println(GPS.longitudeDegrees, 4);
      
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
   }
   */
  }
}
