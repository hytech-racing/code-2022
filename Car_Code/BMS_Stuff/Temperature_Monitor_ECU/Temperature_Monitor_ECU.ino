/*
 * HyTech 2018 Temperature Monitor ECU
 * Init 2017-12-18
 * Configured for Jeff Board rev1
 * Monitors cell temperatures, sends values to BMS over CAN Bus
 */

#include <Arduino.h>
#include <FlexCAN.h>
#include "HyTech17.h"
#include <math.h>
#include <Metro.h>

/*
 * Pin definitions
 */
#define SEG0_C0 A0
#define SEG0_C1 A1
#define SEG0_C2 A2
#define SEG0_C3 A3
#define SEG0_C4 A4
#define SEG1_C0 A5
#define SEG1_C1 A6
#define SEG1_C2 A7
#define SEG1_C3 A8
#define SEG1_C4 A9

/*
 * Constant definitions
 */
#define NUM_THERMISTORS 10

/*
 * Timers
 */
Metro timer_can_update = Metro(1000);

/*
 * Global variables
 */
int16_t raw_temperatures[NUM_THERMISTORS];
BMS_temperatures bms_temperatures;
BMS_detailed_temperatures bms_detailed_temperatures[4];

FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
    pinMode(SEG0_C0, INPUT);
    pinMode(SEG0_C1, INPUT);
    pinMode(SEG0_C2, INPUT);
    pinMode(SEG0_C3, INPUT);
    pinMode(SEG1_C0, INPUT);
    pinMode(SEG1_C1, INPUT);
    pinMode(SEG1_C2, INPUT);
    pinMode(SEG1_C3, INPUT);

    Serial.begin(115200); // Init serial for PC communication
    CAN.begin(); // Init CAN for vehicle communication
    delay(100);
    Serial.println("CAN system and serial communication initialized");

}

void loop() {
    if (timer_can_update.check()) {
        print_uptime();
        read_temperatures();

        bms_temperatures.write(msg.buf);
        msg.id = ID_BMS_TEMPERATURES;
        msg.len = sizeof(CAN_message_bms_temperatures_t);
        CAN.write(msg);

        msg.id = ID_BMS_DETAILED_TEMPERATURES;
        msg.len = sizeof(CAN_message_bms_detailed_temperatures_t);
        for (int i = 0; i < 4; i++) {
            bms_detailed_temperatures[i].write(msg.buf);
            CAN.write(msg);
        }
    }
}

void read_temperatures() {
    raw_temperatures[0] = analogRead(SEG0_C0);
    raw_temperatures[1] = analogRead(SEG0_C1);
    raw_temperatures[2] = analogRead(SEG0_C2);
    raw_temperatures[3] = analogRead(SEG0_C3);
    raw_temperatures[4] = analogRead(SEG0_C4);
    raw_temperatures[5] = analogRead(SEG1_C0);
    raw_temperatures[6] = analogRead(SEG1_C1);
    raw_temperatures[7] = analogRead(SEG1_C2);
    raw_temperatures[8] = analogRead(SEG1_C3);
    raw_temperatures[9] = analogRead(SEG1_C4);

    int32_t average_temperature = 0;

    for (int i = 0; i < NUM_THERMISTORS; i++) {
        double voltage = raw_temperatures[i] / (double) 310; // 0-1023 -> 0-3.3V
        double resistance = (3.3 * 10000 / voltage) - 10000;
        double temperature = 1 / ((1 / (double) 298.15) + (1 / (double) 3984) * (double) log(resistance / 10000)) - (double) 273.15;
        int16_t actual_temperature = temperature * 100;

        Serial.print(i);
        Serial.print(": ");
        Serial.print(temperature, 2);
        Serial.println(" C");

        bms_detailed_temperatures[i / 3].set_temperature(i % 3, actual_temperature); // Populate CAN message struct

        if (actual_temperature > bms_temperatures.get_high_temperature() || i == 0) {
            bms_temperatures.set_high_temperature(actual_temperature);
        }
        if (actual_temperature < bms_temperatures.get_low_temperature() || i == 0) {
            bms_temperatures.set_low_temperature(actual_temperature);
        }
        average_temperature += actual_temperature;
    }
    average_temperature /= NUM_THERMISTORS;
    bms_temperatures.set_average_temperature(average_temperature);

    Serial.print("\nAverage temperature: ");
    Serial.print(bms_temperatures.get_average_temperature() / (double) 100, 2);
    Serial.println(" C");
    Serial.print("Low temperature: ");
    Serial.print(bms_temperatures.get_low_temperature() / (double) 100, 2);
    Serial.println(" C");
    Serial.print("High temperature: ");
    Serial.print(bms_temperatures.get_high_temperature() / (double) 100, 2);
    Serial.println(" C\n\n\n");
}

/*
 * Print ECU uptime
 */
void print_uptime() {
    Serial.print("\n\nECU uptime: ");
    Serial.print(millis() / 1000);
    Serial.print(" seconds (");
    Serial.print(millis() / 1000 / 60);
    Serial.print(" minutes, ");
    Serial.print(millis() / 1000 % 60);
    Serial.println(" seconds)");
}
