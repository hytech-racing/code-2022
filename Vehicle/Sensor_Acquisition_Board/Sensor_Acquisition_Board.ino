#define IS_FRONT (true)

#include "ADC_SPI.h"

#include "SAB_readings.h"
#include "CAN_ID.h"
#include "mcp_can.h"

#include "Metro.h"

// CAN Variables
Metro timer_can_update = Metro(100);
#define SPI_CS 14
MCP_CAN CAN(SPI_CS);

#define ADC_CS 25
#define ALPHA 0.9772                     // parameter for the sowftware filter used on ADC pedal channels
#define ADC_SPI_SPEED 1800000            // max SPI clokc frequency for MCP3208 is 2MHz in ideal conditions
ADC_SPI mcp3208(ADC_CS, ADC_SPI_SPEED);

SAB_readings sab_readings;

/*
 * Variables to store filtered values from ADC channels
 */
float filtered_sensor1_reading{};
float filtered_sensor2_reading{};
float filtered_sensor3_reading{};
float filtered_sensor4_reading{};

#define SENSOR_1_CHANNEL 0
#define SENSOR_2_CHANNEL 1
#define SENSOR_3_CHANNEL 2
#define SENSOR_4_CHANNEL 3

void setup() {
    //Initiallizes CAN
    while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 250K
    {
        delay(200);
    }
}

void loop() { 
    if (timer_can_update.check()){
        uint8_t msg[8] = {0};
        sab_readings.set_sensor_1(filtered_sensor1_reading);
        sab_readings.set_sensor_2(filtered_sensor2_reading);
        sab_readings.set_sensor_3(filtered_sensor3_reading);
        sab_readings.set_sensor_4(filtered_sensor4_reading);
        sab_readings.write(msg);
        #if IS_FRONT
        CAN.sendMsgBuf(ID_SAB_FRONT, 0, sizeof(sab_readings), msg);
        #elif
        CAN.sendMsgBuf(ID_SAB_REAR, 0, sizeof(sab_readings), msg);
        #endif
    }

    filtered_sensor1_reading = ALPHA * filtered_sensor1_reading + (1 - ALPHA) * mcp3208.read_adc(SENSOR_1_CHANNEL);
    filtered_sensor2_reading = ALPHA * filtered_sensor2_reading + (1 - ALPHA) * mcp3208.read_adc(SENSOR_2_CHANNEL);
    filtered_sensor3_reading = ALPHA * filtered_sensor3_reading + (1 - ALPHA) * mcp3208.read_adc(SENSOR_3_CHANNEL);
    filtered_sensor4_reading = ALPHA * filtered_sensor4_reading + (1 - ALPHA) * mcp3208.read_adc(SENSOR_4_CHANNEL);
}