#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "LTC68042.h"
#include <SPI.h>

/********GLOBAL ARRAYS/VARIABLES CONTAINING DATA FROM CHIP**********/
const uint8_t TOTAL_IC = 1;
uint16_t cell_voltages[TOTAL_IC][12]; // contains 12 battery cell voltages
uint16_t aux_voltagess[TOTAL_IC][6]; // contains auxillary pin voltages (not batteries)

/*!<
  The tx_cfg[][6] stores the LTC6804 configuration data that is going to be written 
  to the LTC6804 ICs on the daisy chain. The LTC6804 configuration data that will be
  written should be stored in blocks of 6 bytes. The array should have the following format:
  
 |  tx_cfg[0][0]| tx_cfg[0][1] |  tx_cfg[0][2]|  tx_cfg[0][3]|  tx_cfg[0][4]|  tx_cfg[0][5]| tx_cfg[1][0] |  tx_cfg[1][1]|  tx_cfg[1][2]|  .....    |
 |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
 |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |
 
*/
uint8_t tx_cfg[TOTAL_IC][6]; // data defining how data will be written to daisy chain ICs.

/*!<
  the rx_cfg[][8] array stores the data that is read back from a LTC6804-1 daisy chain. 
  The configuration data for each IC  is stored in blocks of 8 bytes. Below is an table illustrating the array organization:

|rx_config[0][0]|rx_config[0][1]|rx_config[0][2]|rx_config[0][3]|rx_config[0][4]|rx_config[0][5]|rx_config[0][6]  |rx_config[0][7] |rx_config[1][0]|rx_config[1][1]|  .....    |
|---------------|---------------|---------------|---------------|---------------|---------------|-----------------|----------------|---------------|---------------|-----------|
|IC1 CFGR0      |IC1 CFGR1      |IC1 CFGR2      |IC1 CFGR3      |IC1 CFGR4      |IC1 CFGR5      |IC1 PEC High     |IC1 PEC Low     |IC2 CFGR0      |IC2 CFGR1      |  .....    |
*/
uint8_t rx_cfg[TOTAL_IC][8];


void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    LTC6804_initialize();
    init_cfg();
}

/*
 * Continuously poll voltages and print them to the Serial Monitor.
 */
void loop() {
  // put your main code here, to run repeatedly:
  pollVoltage();
  printCells();
}

/*!***********************************
 \brief Initializes the configuration array
 **************************************/
void init_cfg()
{
    for(int i = 0; i<TOTAL_IC;i++)
    {
        tx_cfg[i][0] = 0xFE;
        tx_cfg[i][1] = 0x00 ; 
        tx_cfg[i][2] = 0x00 ;
        tx_cfg[i][3] = 0x00 ; 
        tx_cfg[i][4] = 0x00 ;
        tx_cfg[i][5] = 0x00 ;
    }
}

void pollVoltage() {
    Serial.println("Polling Voltages...");
    /*
     * Difference between wakeup_sleep and wakeup_idle
     * wakeup_sleep wakes up the LTC6804 from sleep state
     * wakeup_idle wakes up the isoSPI port.
     */
    wakeup_sleep();
    LTC6804_wrcfg(TOTAL_IC, tx_cfg);
    wakeup_idle();
    LTC6804_adcv();
    delay(10);
    wakeup_idle();
    uint8_t error = LTC6804_rdcv(0, TOTAL_IC, cell_voltages); // calls the chip to read and store voltage values in the array.
    if (error == -1) {
        Serial.println("A PEC error was detected in voltage data");
    }
    print_cells(); // prints the cell voltages to Serial.
    delay(500);
}

void printCells() {
    for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++) {
        Serial.print("IC: ");
        Serial.println(current_ic+1);
        for (int i = 0; i < 12; i++) {
            Serial.print("C"); Serial.print(i+1); Serial.print(": ");
            Serial.println(cell_voltages[current_ic][i] * 0.0001, 4);
        }
        Serial.println();
    }
}

