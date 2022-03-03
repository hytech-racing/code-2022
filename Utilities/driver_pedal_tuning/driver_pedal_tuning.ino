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

#include "../../Vehicle/Main_Control_Unit/drivers.h"

#define DRIVER DAVID

#include "../../Vehicle/Main_Control_Unit/MCU_rev10_dfs.h"
#include "../../Vehicle/Main_Control_Unit/driver_constants.h"

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

    static bool raw = false;
    static bool apps = false;
    if(Serial.available()){
        String a = Serial.readString();
        if (a == "on" || a == "on\n") {
          Serial.println("raw print turned on");
          raw = true;
        }
        else if (a == "apps" || a == "apps\n"){
          Serial.println("apps print turned on");
            apps = true;
        }
        else if (a == "off" || a == "off\n"){
          Serial.println("Print turned off");
          raw = false;
          apps = false;
        }
    }
    if (raw && timer_serial_print.check()) {
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
      if (filtered_accel2_reading < MIN_ACCELERATOR_PEDAL_2) {
          Serial.print("Accel 2 is below minimum threshold of "); Serial.print(MIN_ACCELERATOR_PEDAL_2); Serial.println(". Fault.");
      }
      if (filtered_accel2_reading >= START_ACCELERATOR_PEDAL_2 && filtered_accel2_reading <= END_ACCELERATOR_PEDAL_2) {
          Serial.print("Accel 2 is within valid torque range of "); Serial.print(START_ACCELERATOR_PEDAL_2); Serial.print(" to ");
          Serial.print(END_ACCELERATOR_PEDAL_2); Serial.println(".");
      }
      if (filtered_accel2_reading >o MAX_ACCELERATOR_PEDAL_2) {
          Serial.print("Accel 2 is above maximum threshold of "); Serial.print(MAX_ACCELERATOR_PEDAL_2); Serial.println(". Fault.");
      }
    }
    if ( apps && timer_serial_print.check()){
        if (filtered_accel1_reading < MIN_ACCELERATOR_PEDAL_1 || filtered_accel1_reading > MAX_ACCELERATOR_PEDAL_1) {
            Serial.print("T.4.2.10 1\t");
            Serial.println("Accel 1 is out of range");
        }
        else if (filtered_accel2_reading < MAX_ACCELERATOR_PEDAL_2 ||filtered_accel2_reading > MIN_ACCELERATOR_PEDAL_2) {
            Serial.print("T.4.2.10 2\t");
            Serial.println("Accel 2 is out of range");
        }
        // check that the pedals are reading within 10% of each other
        // sum of the two readings should be within 10% of the average travel
        // T.4.2.4
        else if ((filtered_accel1_reading - (4096 - filtered_accel2_reading)) >
                    (END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1 + START_ACCELERATOR_PEDAL_2 - END_ACCELERATOR_PEDAL_2)/20 ){
            Serial.println("T.4.2.4");
            Serial.printf("computed - %f\n", filtered_accel1_reading - (4096 - filtered_accel2_reading));
            Serial.printf("standard - %d\n", (END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1 + START_ACCELERATOR_PEDAL_2 - END_ACCELERATOR_PEDAL_2)/20);
        }

        // BSE check
        // EV.5.6
        // FSAE T.4.3.4
        if (filtered_brake1_reading < 409 || filtered_brake1_reading > 3687) {
            Serial.println("T.4.3.4 brake out of range");
        }

        // FSAE EV.5.7
        // APPS/Brake Pedal Plausability Check
        // if  (
        //         (
        //             (filtered_accel1_reading > ((END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1)/4 + START_ACCELERATOR_PEDAL_1))
        //             ||
        //             (filtered_accel2_reading < ((END_ACCELERATOR_PEDAL_2 - START_ACCELERATOR_PEDAL_2)/4 + START_ACCELERATOR_PEDAL_2))
        //         )
        //         && filtered_brake1_reading > BRAKE_ACTIVE
        //     )
        // {
        //     Serial.println("EV.5.7 APPS/Brake Pedal implausability entering");
        // }
        // else if
        // (
        //     (filtered_accel1_reading < ((END_ACCELERATOR_PEDAL_1 - START_ACCELERATOR_PEDAL_1)/20 + START_ACCELERATOR_PEDAL_1))
        //     &&
        //     (filtered_accel2_reading > ((END_ACCELERATOR_PEDAL_2 - START_ACCELERATOR_PEDAL_2)/20 + START_ACCELERATOR_PEDAL_2))
        // )
        // {
        //     Serial.println("EV.5.7 APPS/Brake Pedal implausability exiting");
        // }
    }
}
