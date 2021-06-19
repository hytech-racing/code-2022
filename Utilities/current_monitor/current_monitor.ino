/*
    Current-logging program for use with coloumb counting.
    This code requires a board with a Teensy 3.2 and an MCP3208 ADC.
    It is configured for use with an ISB-100-A-604 current sensor.

    Created May 2019 (Nathan Cheek - ncheek@gatech.edu)
*/

#include <ADC_SPI.h>
#include <Metro.h>

#define ADC_SPI_CS 9    // Chip Select
#define ADC_CUR_CH 0    // Current sensor channel
#define TIMESTEP 100    // Datalog timestemp (milliseconds)
#define CALIBRATION_OFFSET 2 // Optionally calibrate the sensor so it shows 0A when no current is flowing

//////////Create an adc object
ADC_SPI adc;
//////////Create a metro timer object
Metro timer  = Metro(TIMESTEP, 1); // return true based on timestep period; ignore missed calls;
//////////Create voltage reading sum for average
uint32_t adc_sum = 0;
uint16_t adc_read_count = 0;

void setup() {
    adc = ADC_SPI(ADC_SPI_CS);
    Serial.begin(115200);
    delay(100);
    Serial.println("timestamp (ms),current (A),raw average,sample count");
}

void loop() {
    // Read raw voltage and add to sum
    adc_sum += adc.read_adc(ADC_CUR_CH);
    adc_sum += CALIBRATION_OFFSET;
    adc_read_count++;

    if (timer.check()) { // Calculate average voltage, calculate current, then print to serial
        double average_voltage = adc_sum / (double) adc_read_count;
        double cur = process_current(average_voltage);
        Serial.print(millis());
        Serial.print(",");
        Serial.print(cur,2);
        Serial.print(",");
        Serial.print(average_voltage, 0);
        Serial.print(",");
        Serial.println(adc_read_count);
        adc_sum = 0;
        adc_read_count = 0;
    }
}

double process_current(double adc_reading) {
    /*
     * Current sensor: ISB-100-A-604
     * Maximum positive current (100A) corresponds to 4.5V signal
     * Maximum negative current (-100A) corresponds to 0.5V signal
     * 0A current corresponds to 2.5V signal
     *
     * voltage = read_adc() * 5 / 4095
     * current = (voltage - 2.5) * 100 / 2
     */
    double voltage = adc_reading / (double) 819;
    double current = (voltage - 2.5) * (double) 50;
    return current;
}
