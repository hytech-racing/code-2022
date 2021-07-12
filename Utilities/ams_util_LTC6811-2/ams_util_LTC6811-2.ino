/*
   HyTech 2021 AMS Basic Monitor
   Init 2021-7-11
   Configured for Teensy with added testing functionality
   Based off of HyTech 2018 BMS Basic Monitor
   Modified to support Teensy, and work with 4 parallel ICs
*/

/*!
  LTC6811-2: Battery stack monitor

  @verbatim
  NOTES
  Setup:
   Set the terminal baud rate to 115200 and select the newline terminator.
   Ensure all jumpers on the demo board are installed in their default positions from the factory.
   Refer to Demo Manual D1894B.

  USER INPUT DATA FORMAT:
  decimal : 1024
  hex     : 0x400
  octal   : 02000  (leading 0)
  binary  : B10000000000
  float   : 1024.0

  @endverbatim

  http://www.linear.com/product/LTC6804-1

  http://www.linear.com/product/LTC6804-1#demoboards

  REVISION HISTORY
  $Revision: 4432 $
  $Date: 2015-11-30 14:03:02 -0800 (Mon, 30 Nov 2015) $

  Copyright (c) 2013, Linear Technology Corp.(LTC)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  The views and conclusions contained in the software and documentation are those
  of the authors and should not be interpreted as representing official policies,
  either expressed or implied, of Linear Technology Corp.

  The Linear Technology Linduino is not affiliated with the official Arduino team.
  However, the Linduino is only possible because of the Arduino team's commitment
  to the open-source community.  Please, visit http://www.arduino.cc and
  http://store.arduino.cc , and consider a purchase that will help fund their
  ongoing work.

  Copyright 2013 Linear Technology Corp. (LTC)
*/

#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include <ADC_SPI.h>
#include "Linduino.h"
#include "UserInterface.h"
#include "LTC6811.h"
#include "LT_SPI.h"

const uint8_t TOTAL_IC = 16;

/** All data packaged into an array of cell_asic structs
  * ic_data[index].config.tx_data is used to write config to the cells
  * ic_data[index].config.rx_data is used to read config from the cells
  * ic_data[index].cells contains read cell voltage data
  * ic_data[index].aux contains read aux voltage data
*/
cell_asic ic_data[TOTAL_IC];



void setup() {
  //pinMode(10, OUTPUT);  
  Serial.begin(115200);
  //adc = ADC_SPI(9);
  LTC6811_init_cfg(TOTAL_IC, ic_data);
  quikeval_SPI_connect();
  spi_enable(SPI_CLOCK_DIV16);
  init_cfg();
  print_menu();

}

void loop() {
  //read and run the user's ams command
  if(Serial.available()) {
    uint32_t user_command = read_int();
    Serial.println(user_command);
    run_command(user_command);
  }
}


/*!*****************************************
  \brief executes the user inputted command

  Menu Entry 1: Write Configuration \n
   Writes the configuration register of the LTC6804. This command can be used to turn on the reference
   and increase the speed of the ADC conversions.

  Menu Entry 2: Read Configuration \n
   Reads the configuration register of the LTC6804, the read configuration can differ from the written configuration.
   The GPIO pins will reflect the state of the pin

  Menu Entry 3: Start Cell voltage conversion \n
   Starts a LTC6804 cell channel adc conversion.

  Menu Entry 4: Read cell voltages
    Reads the LTC6804 cell voltage registers and prints the results to the serial port.

  Menu Entry 5: Start Auxiliary voltage conversion
    Starts a LTC6804 GPIO channel adc conversion.

  Menu Entry 6: Read Auxiliary voltages
    Reads the LTC6804 axiliary registers and prints the GPIO voltages to the serial port.

  Menu Entry 7: Start cell voltage measurement loop
    The command will continuously measure the LTC6804 cell voltages and print the results to the serial port.
    The loop can be exited by sending the MCU a 'm' character over the serial link.

  Menu Entry 8: Start aux voltage measurement loop
    Continuously measure aux voltages.

  Menu Entry 9: Start aux voltage measurement with register clearing loop
    Continously measure aux voltages, then clear registers, to test if new values are being measured.

  Menu Entry 10: Start cell balancing loop
    Continuously loops through cells, balancing one cell at a time.

*******************************************/

void run_command(uint16_t cmd) {
  int8_t error = 0;

  char input = 0;
  switch(cmd) {
    case 1:
      wakeup_sleep(TOTAL_IC);
      LTC6811_wrcfg(TOTAL_IC, ic_data);
      print_config();
      break;
    case 2:
      wakeup_sleep(TOTAL_IC);
      error = LTC6811_rdcfg(TOTAL_IC, ic_data);
      if(error == -1) {
        Serial.println("A PEC error was detected in the received data");
      }
      print_rxconfig();
      break;
    case 3:
      wakeup_sleep(TOTAL_IC);
      //see datasheet tables 38 and 39 for command codes
      LTC6811_adcv(MD_7KHZ_3KHZ, DCP_DISABLED, CELL_CH_ALL); //start adc in normal mode, discharge not permitted, measure all cells 
      delay(3);
      Serial.println("cell conversion complete");
      Serial.println();
      break;
    case 4:
      wakeup_sleep(TOTAL_IC);
      error = LTC6811_rdcv(0, TOTAL_IC, ic_data);
      if(error == -1) {
        Serial.println("A PEC error was detected in the received data");
      }
      print_cells();
      break;
    case 5:
      wakeup_sleep(TOTAL_IC);
      LTC6811_adax(MD_7KHZ_3KHZ, AUX_CH_ALL); //adc normal mode, all GPIO
      delay(3);
      Serial.println("aux conversion completed");
      Serial.println();
      break;
    case 6:
      wakeup_sleep(TOTAL_IC);
      error = LTC6811_rdaux(0, TOTAL_IC, ic_data);
      if(error == -1) {
        Serial.println("A PEC error was detected in the received data");
      }
      print_aux();
      break;


    default:
      Serial.println("Incorrect Operation");
      break;
  }
}


/*!***********************************
  \brief Initializes the configuration array
  See Table 40 for the Configuration Register Group
  See Table 52 for Memory Bit Descriptions
 **************************************/
void init_cfg()
{
  for (int i = 0; i < TOTAL_IC; i++)
  {
    //LTC6811_set_cfgr(i, ic_data, DATA GOES HERE SEE GITHUB);
  }
}

/*!************************************************************
  \brief Prints Cell Voltage Codes to the serial port
 *************************************************************/
void print_cells()
{


  for (int current_ic = 0 ; current_ic < TOTAL_IC; current_ic++)
  {
    Serial.print(" IC ");
    Serial.print(current_ic, DEC);
    for (int i = 0; i < 12; i++)
    {
      Serial.print(" C");
      Serial.print(i, DEC);
      Serial.print(":");
      Serial.print(ic_data[current_ic].cells.c_codes[i] * 0.0001, 4);
      Serial.print(",");
    }
    Serial.println();
  }
  Serial.println();
}

void print_aux()
{

  for (int current_ic = 0 ; current_ic < TOTAL_IC; current_ic++)
  {
    Serial.print(" IC ");
    Serial.print(current_ic, DEC);
    for (int i = 0; i < 5; i++)
    {
      Serial.print(" GPIO-");
      Serial.print(i, DEC);
      Serial.print(":");
      Serial.print(ic_data[current_ic].aux.a_codes[i] * 0.0001, 4);
      Serial.print(",");
    }
    Serial.print(" Vref2");
    Serial.print(":");
    Serial.print(ic_data[current_ic].aux.a_codes[5] * 0.0001, 4);
    Serial.println();
  }
  Serial.println();
}


/*!*********************************
  \brief Prints the main menu
***********************************/
void print_menu() {
  Serial.println("Please enter LTC6811 Command");
  Serial.println("Write Configuration: 1");
  Serial.println("Read Configuration: 2");
  Serial.println("Start Cell Voltage Conversion: 3");
  Serial.println("Read Cell Voltages: 4");
  Serial.println("Start Aux Voltage Conversion: 5");
  Serial.println("Read Aux Voltages: 6");
  Serial.println("Loop Cell Voltages: 7");
  Serial.println("Loop Aux Voltages: 8");
  Serial.println("Loop Aux Voltages, clearing after every measurement: 9");
  Serial.println("Loop Cell Balancing simulatenously for all IC per Battery Cell: 10");
  Serial.println("Loop Cell Balancing per individual Battery Cells: 11");
  Serial.println("Please enter command: ");
  Serial.println();
}

/*!******************************************************************************
  \brief Prints the Configuration data that is going to be written to the LTC6804
  to the serial port.
 ********************************************************************************/
void print_config()
{
  int cfg_pec;

  Serial.println("Written Configuration: ");
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++)
  {
    Serial.print(" IC ");
    Serial.print(current_ic, DEC);
    Serial.print(": ");
    Serial.print("0x");
    serial_print_hex(ic_data[current_ic].config.tx_data[0]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.tx_data[1]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.tx_data[2]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.tx_data[3]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.tx_data[4]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.tx_data[5]);
    Serial.print(", Calculated PEC: 0x");
    cfg_pec = pec15_calc(6, ic_data[current_ic].config.tx_data);
    serial_print_hex((uint8_t)(cfg_pec >> 8));
    Serial.print(", 0x");
    serial_print_hex((uint8_t)(cfg_pec));
    Serial.println();
  }
  Serial.println();
}

/*!*****************************************************************
  \brief Prints the Configuration data that was read back from the
  LTC6804 to the serial port.
 *******************************************************************/
void print_rxconfig()
{
  Serial.println("Received Configuration ");
  for (int current_ic = 0; current_ic < TOTAL_IC; current_ic++)
  {
    Serial.print(" IC ");
    Serial.print(current_ic, DEC);
    Serial.print(": 0x");
    serial_print_hex(ic_data[current_ic].config.rx_data[0]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.rx_data[1]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.rx_data[2]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.rx_data[3]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.rx_data[4]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.rx_data[5]);
    Serial.print(", Received PEC: 0x");
    serial_print_hex(ic_data[current_ic].config.rx_data[6]);
    Serial.print(", 0x");
    serial_print_hex(ic_data[current_ic].config.rx_data[7]);
    Serial.println();
  }
  Serial.println();
}

void serial_print_hex(uint8_t data)
{
  if (data < 16)
  {
    Serial.print("0");
    Serial.print((byte)data, HEX);
  }
  else
    Serial.print((byte)data, HEX);
}