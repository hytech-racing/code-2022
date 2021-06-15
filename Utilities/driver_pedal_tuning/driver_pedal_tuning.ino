#include <stdint.h>

#include "ADC_SPI.h"
#include "HyTech_FlexCAN.h"
#include "HyTech_CAN.h"
#include "kinetis_flexcan.h"
#include "Metro.h"

float filtered_accel1_reading{};
float filtered_accel2_reading{};
float filtered_brake1_reading{};
float filtered_brake2_reading{};

#include "../../Vehicle/Main_Control_Unit_rev10_testing/drivers.h"

#define DRIVER DAVID

#include "../../Vehicle/Main_Control_Unit_rev10_testing/MCU_rev10_dfs.h"
ADC_SPI ADC(ADC_CS, ADC_SPI_SPEED);

Metro timer_serial_print(500);

void setup() {
    Serial.begin(115200);
}

void loop() {
    /* Filter ADC readings */
    filtered_accel1_reading = ALPHA * filtered_accel1_reading + (1 - ALPHA) * ADC.read_adc(ADC_ACCEL_1_CHANNEL);
    filtered_accel2_reading = ALPHA * filtered_accel2_reading + (1 - ALPHA) * ADC.read_adc(ADC_ACCEL_2_CHANNEL);
    filtered_brake1_reading = ALPHA * filtered_brake1_reading + (1 - ALPHA) * ADC.read_adc(ADC_BRAKE_1_CHANNEL);
    filtered_brake2_reading = ALPHA * filtered_brake2_reading + (1 - ALPHA) * ADC.read_adc(ADC_BRAKE_2_CHANNEL);

    if (timer_serial_print.check()){
      Serial.println("\n\n");
      Serial.print("ACCEL 1: "); Serial.println(filtered_accel1_reading);
      Serial.print("ACCEL 2: "); Serial.println(filtered_accel2_reading);
      Serial.print("BRAKE 1: "); Serial.println(filtered_brake1_reading);
      Serial.print("BRAKE 2: "); Serial.println(filtered_brake2_reading);
  
      if (filtered_brake1_reading > BRAKE_ACTIVE)
          Serial.println("Brake pedal active");
  
      Serial.println("----------------------------\nAccel 1 Check");
      if (filtered_accel1_reading < MIN_ACCELERATOR_PEDAL_1) {
          Serial.print("Accel 1 is below minimum threshold of  "); Serial.print(MIN_ACCELERATOR_PEDAL_1); Serial.println(". Fault.");
      }
      if (filtered_accel1_reading >= START_ACCELERATOR_PEDAL_1 && filtered_accel1_reading <= END_ACCELERATOR_PEDAL_1) {
          Serial.print("Accel 1 is within valid torque range of "); Serial.print(START_ACCELERATOR_PEDAL_1); Serial.print(" to ");
          Serial.print(END_ACCELERATOR_PEDAL_1); Serial.println(".");
      }
      if (filtered_accel1_reading > MAX_ACCELERATOR_PEDAL_1) {
          Serial.print("Accel 1 is above maximum threshold of "); Serial.print(MAX_ACCELERATOR_PEDAL_1); Serial.println(". Fault.");
      }
  
      Serial.println("----------------------------\nAccel 2 Check");
      if (filtered_accel2_reading > MIN_ACCELERATOR_PEDAL_2) {
          Serial.print("Accel 2 is below minimum threshold of "); Serial.print(MIN_ACCELERATOR_PEDAL_2); Serial.println(". Fault.");
      }
      if (filtered_accel2_reading <= START_ACCELERATOR_PEDAL_2 && filtered_accel2_reading >= END_ACCELERATOR_PEDAL_2) {
          Serial.print("Accel 2 is within valid torque range of "); Serial.print(START_ACCELERATOR_PEDAL_2); Serial.print(" to ");
          Serial.print(END_ACCELERATOR_PEDAL_2); Serial.println(".");
      }
      if (filtered_accel2_reading < MAX_ACCELERATOR_PEDAL_2) {
          Serial.print("Accel 2 is above maximum threshold of "); Serial.print(MAX_ACCELERATOR_PEDAL_2); Serial.println(". Fault.");
      }
    }
}
