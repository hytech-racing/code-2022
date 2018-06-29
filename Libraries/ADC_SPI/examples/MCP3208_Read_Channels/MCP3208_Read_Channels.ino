#include <ADC_SPI.h>

ADC_SPI adc;

void setup() {
  // adc = ADC_SPI(9); // Use if CS is not on pin 10
  adc = ADC_SPI();
  Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < 8; i++) {
    int val = adc.read_adc(i);
    Serial.print("C");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(val);
    Serial.print(" - ");
  }
  Serial.println();
  delay(1000);
}
