#include <ADC_SPI.h>

ADC_SPI adc;
int readings[15];

void setup() {
  // adc = ADC_SPI(9); // Use if CS is not on pin 10
  adc = ADC_SPI(9);
  Serial.begin(115200);
  pinMode(A9, OUTPUT);
  analogWriteFrequency(A9, 80);
  analogWrite(A9, 128);
  //digitalWrite(A9, HIGH);
}

void loop() {
    int val = 0;
    for (int i = 0; i < 10; i++) {
      val += adc.read_adc(2);
    }
    val /= 10;
    //int val = adc.read_adc(1);
    Serial.println(val);
//    Serial.println(adc.read_adc(0));
    /*for (int i = 0; i < 15; i++) {
      readings[i] = adc.read_adc(1);
    }
    sort_ints(readings, 15);
    Serial.println(readings[7]);*/
}
/* Comparison function. Receives two generic (void) pointers to the items under comparison. */
int compare_ints(const void *p, const void *q) {
    int x = *(const int *)p;
    int y = *(const int *)q;

    /* Avoid return x - y, which can cause undefined behaviour
       because of signed integer overflow. */
    if (x < y)
        return -1;  // Return -1 if you want ascending, 1 if you want descending order. 
    else if (x > y)
        return 1;   // Return 1 if you want ascending, -1 if you want descending order. 

    return 0;
}

/* Sort an array of n integers, pointed to by a. */
void sort_ints(int *a, size_t n) {
    qsort(a, n, sizeof *a, &compare_ints);
}
