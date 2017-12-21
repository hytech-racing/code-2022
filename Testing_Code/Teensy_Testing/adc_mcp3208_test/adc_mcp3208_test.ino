// Credit to Arduino Playground
// https://playground.arduino.cc/Code/MCP3208

#define SELPIN A0 //Selection Pin 
#define DATAOUT 1//MOSI 
#define DATAIN  0//MISO 
#define SPICLOCK  13//Clock 
int readvalue; 

void setup(){
 //set pin modes 
 pinMode(SELPIN, OUTPUT);
 pinMode(DATAOUT, OUTPUT);
 pinMode(DATAIN, INPUT);
 pinMode(SPICLOCK, OUTPUT);
 //disable device to start with 
 digitalWrite(SELPIN,HIGH);
 digitalWrite(DATAOUT,LOW);
 digitalWrite(SPICLOCK,LOW);
 
 Serial.begin(9600);
}

int read_adc(int channel) {
  int adcvalue = 0;
  byte commandbits = B11000000; //command bits - start, mode, chn (3), dont care (3)

  //allow channel selection
  commandbits|=(channel<<3);

  digitalWrite(SELPIN,LOW); //Select adc
  // setup bits to be written
  for (int i=7; i>=3; i--){
    digitalWrite(DATAOUT,commandbits&1<<i);
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);    
  }

  digitalWrite(SPICLOCK,HIGH);    //ignores 2 null bits
  digitalWrite(SPICLOCK,LOW);
  digitalWrite(SPICLOCK,HIGH);  
  digitalWrite(SPICLOCK,LOW);

  //read bits from adc
  for (int i=11; i>=0; i--){
    adcvalue+=digitalRead(DATAIN)<<i;
    //cycle clock
    digitalWrite(SPICLOCK,HIGH);
    digitalWrite(SPICLOCK,LOW);
  }
  digitalWrite(SELPIN, HIGH); //turn off device
  return adcvalue;
}

void loop() { 
 readvalue = read_adc(0);
 Serial.print("CH0: ACCEL 1: ");
 Serial.println(readvalue, DEC);
 readvalue = read_adc(1);
 Serial.print("CH1: ACCEL 2: ");
 Serial.println(readvalue, DEC);
 readvalue = read_adc(2);
 Serial.print("CH2: BRAKE: ");
 Serial.println(readvalue, DEC);
 Serial.println(" "); 
 delay(250); 
} 
