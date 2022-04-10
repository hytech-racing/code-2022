#define LED_INERTIA A5

bool blink = true;

void setup() {
  pinMode(LED_INERTIA, OUTPUT);
}

void loop() {
  if (blink) {
    digitalWrite(LED_INERTIA, HIGH);
  } else {
    digitalWrite(LED_INERTIA, LOW);
  }                                   
  delay(500);
  blink = !blink;
}
