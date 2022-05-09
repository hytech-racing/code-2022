void setup() {
  Serial.begin(115200);

  
}

void loop() {
  Serial.println("------------------------------------------------------------------------------------------------------------------------------------------------------------");
  Serial.println("\t\tAC VOLTAGE\t\tOUTPUT VOLTAGE\t\tOUTPUT CURRENT\t\tFLAGS");
  Serial.print("\t\t 120.0");
  Serial.print(" V\t\t   ");
  Serial.print("350.0");
  Serial.print(" V\t\t    ");
  Serial.print("30.0");
  Serial.print(" A\t\t  ");
  Serial.print("0");
  Serial.println();
}
