/*
   HyTech 2018 BMS Basic Monitor
   Init 2018-01-23
   Configured for Teensy with added testing functionality
   Based off of DC1942.ino from Linear
   Modified to support Teensy, and work with 4 parallel ICs
*/

/*!
  DC1942B
  LTC6804-2: Battery stack monitor

  @verbatim
  NOTES
  Setup:
   Set the terminal baud rate to 115200 and select the newline terminator.
   Ensure all jumpers on the demo board are installed in their default positions from the factory.
   Refer to Demo Manual D1894B.


  Menu Entry 1: Write Configuration
   Writes the configuration register of the LTC6804. This command can be used to turn on the reference.

  Menu Entry 2: Read Configuration
   Reads the configuration register of the LTC6804, the read configuration can differ from the written configuration.
   The GPIO pins will reflect the state of the pin

  Menu Entry 3: Start Cell voltage conversion
    Starts a LTC6804 cell channel adc conversion.

  Menu Entry 4: Read cell voltages
    Reads the LTC6804 cell voltage registers and prints the results to the serial port.

  Menu Entry 5: Start Auxiliary voltage conversion
    Starts a LTC6804 GPIO channel adc conversion.

  Menu Entry 6: Read Auxiliary voltages
    Reads the LTC6804 axiliary registers and prints the GPIO voltages to the serial port.

  Menu Entry 7: Start cell voltage measurement loop
    The command will continuously measure the LTC6804 cell voltages and MCP3208 ADC readings and print the results to the serial port.
    The loop can be exited by sending the MCU a 'm' character over the serial link.

  Menu Entry 8: Start aux voltage measurement loop
    Continuously measure aux voltages.

  Menu Entry 9: Start aux voltage measurement with register clearing loop
    Continously measure aux voltages, then clear registers, to test if new values are being measured.

  Menu Entry 10: Start cell balancing loop
    Continuously loops through cells, balancing one cell at a time.

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


/*! @file
    @ingroup LTC68042
*/

#include <Arduino.h>
#include <stdint.h>
#include "Linduino.h"
#include "LT_SPI.h"
#include "UserInterface.h"
#include "LTC68042.h"
#include <SPI.h>
#include <ADC_SPI.h>

ADC_SPI adc;

const uint8_t TOTAL_IC = 16;//!<number of ICs in the isoSPI network LTC6804-2 ICs must be addressed in ascending order starting at 0.

/******************************************************
 *** Global Battery Variables received from 6804 commands
  These variables store the results from the LTC6804
  register reads and the array lengths must be based
  on the number of ICs on the stack
 ******************************************************/
uint16_t cell_codes[TOTAL_IC][12];
/*!<
  The cell codes will be stored in the cell_codes[][12] array in the following format:

  |  cell_codes[0][0]| cell_codes[0][1] |  cell_codes[0][2]|    .....     |  cell_codes[0][11]|  cell_codes[1][0] | cell_codes[1][1]|  .....   |
  |------------------|------------------|------------------|--------------|-------------------|-------------------|-----------------|----------|
  |IC1 Cell 1        |IC1 Cell 2        |IC1 Cell 3        |    .....     |  IC1 Cell 12      |IC2 Cell 1         |IC2 Cell 2       | .....    |
****/

uint16_t aux_codes[TOTAL_IC][6];
/*!<
  The GPIO codes will be stored in the aux_codes[][6] array in the following format:

  |  aux_codes[0][0]| aux_codes[0][1] |  aux_codes[0][2]|  aux_codes[0][3]|  aux_codes[0][4]|  aux_codes[0][5]| aux_codes[1][0] |aux_codes[1][1]|  .....    |
  |-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|-----------------|---------------|-----------|
  |IC1 GPIO1        |IC1 GPIO2        |IC1 GPIO3        |IC1 GPIO4        |IC1 GPIO5        |IC1 Vref2        |IC2 GPIO1        |IC2 GPIO2      |  .....    |
*/

uint8_t tx_cfg[TOTAL_IC][6];
/*!<
  The tx_cfg[][6] stores the LTC6804 configuration data that is going to be written
  to the LTC6804 ICs on the daisy chain. The LTC6804 configuration data that will be
  written should be stored in blocks of 6 bytes. The array should have the following format:

  |  tx_cfg[0][0]| tx_cfg[0][1] |  tx_cfg[0][2]|  tx_cfg[0][3]|  tx_cfg[0][4]|  tx_cfg[0][5]| tx_cfg[1][0] |  tx_cfg[1][1]|  tx_cfg[1][2]|  .....    |
  |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
  |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |

*/

uint8_t rx_cfg[TOTAL_IC][8];
/*!<
  the rx_cfg[][8] array stores the data that is read back from a LTC6804-1 daisy chain.
  The configuration data for each IC  is stored in blocks of 8 bytes. Below is an table illustrating the array organization:

  |rx_config[0][0]|rx_config[0][1]|rx_config[0][2]|rx_config[0][3]|rx_config[0][4]|rx_config[0][5]|rx_config[0][6]  |rx_config[0][7] |rx_config[1][0]|rx_config[1][1]|  .....    |
  |---------------|---------------|---------------|---------------|---------------|---------------|-----------------|----------------|---------------|---------------|-----------|
  |IC1 CFGR0      |IC1 CFGR1      |IC1 CFGR2      |IC1 CFGR3      |IC1 CFGR4      |IC1 CFGR5      |IC1 PEC High     |IC1 PEC Low     |IC2 CFGR0      |IC2 CFGR1      |  .....    |
*/

uint8_t balance_cell_index = 0;
uint8_t IC_index = 0;

/*!**********************************************************************
  \brief  Inititializes hardware and variables
 ***********************************************************************/
void setup()
{
  pinMode(10, OUTPUT); // Enable CS pin for Teensy compatibility
  Serial.begin(115200);
  adc = ADC_SPI(9);
  LTC6804_initialize();  //Initialize LTC6804 hardware
  init_cfg();        //initialize the 6804 configuration array to be written
  print_menu();
}

/*!*********************************************************************
  \brief main loop

***********************************************************************/
void loop()
{

  if (Serial.available())           // Check for user input
  {
    uint32_t user_command;
    user_command = read_int();      // Read the user command
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
void run_command(uint16_t cmd)
{
  int8_t error = 0;

  char input = 0;
  switch (cmd)
  {

    case 1:
      wakeup_sleep();
      LTC6804_wrcfg(TOTAL_IC, tx_cfg);
      print_config();
      break;

    case 2:
      wakeup_sleep();
      error = LTC6804_rdcfg(TOTAL_IC, rx_cfg);
      if (error == -1)
      {
        Serial.println("A PEC error was detected in the received data");
      }
      print_rxconfig();
      break;

    case 3:
      wakeup_sleep();
      LTC6804_adcv();
      delay(3);
      Serial.println("cell conversion completed");
      Serial.println();
      break;

    case 4:
      wakeup_sleep();
      error = LTC6804_rdcv(0, TOTAL_IC, cell_codes); // Set to read back all cell voltage registers
      if (error == -1)
      {
        Serial.println("A PEC error was detected in the received data");
      }
      print_cells();
      break;

    case 5:
      wakeup_sleep();
      LTC6804_adax();
      delay(3);
      Serial.println("aux conversion completed");
      Serial.println();
      break;

    case 6:
      wakeup_sleep();
      error = LTC6804_rdaux(0, TOTAL_IC, aux_codes); // Set to read back all aux registers
      if (error == -1)
      {
        Serial.println("A PEC error was detected in the received data");
      }
      print_aux();
      break;

    case 7:
      Serial.println("transmit 'm' to quit");
      wakeup_sleep();
      LTC6804_wrcfg(TOTAL_IC, tx_cfg);
      while (input != 'm')
      {
        if (Serial.available() > 0)
        {
          input = read_char();
        }
        wakeup_idle();
        LTC6804_adcv();
        delay(10);
        wakeup_idle();
        error = LTC6804_rdcv(0, TOTAL_IC, cell_codes);
        if (error == -1)
        {
          Serial.println("A PEC error was detected in the received data");
        }
        print_cells();
        Serial.print("[MCP3208 ADC] ");
        for (int i = 0; i < 8; i++) {
          int val = adc.read_adc(i);
          Serial.print("C");
          Serial.print(i);
          Serial.print(": ");
          Serial.print(val);
          Serial.print('\t');
        }
        Serial.println();
        Serial.println();
        spi_enable(SPI_CLOCK_DIV16); // Set SPI back to 1MHz for next isoSPI call; It might be best practice to put this right before each isoSPI call
        delay(500);
      }
      print_menu();
      break;

    case 8:
      Serial.println("transmit 'm' to quit");
      wakeup_sleep();
      LTC6804_wrcfg(TOTAL_IC, tx_cfg);
      delay(5);
      while (input != 'm')
      {
        if (Serial.available() > 0)
        {
          input = read_char();
        }
        LTC6804_adax();
        delay(10);
        error = LTC6804_rdaux(0, TOTAL_IC, aux_codes); // Set to read back all aux registers
        if (error == -1)
        {
          Serial.println("A PEC error was detected in the received data");
        }
        print_aux();
        delay(1000);
      }
      print_menu();
      break;

    case 9:
      Serial.println("transmit 'm' to quit");
      wakeup_sleep();
      LTC6804_wrcfg(TOTAL_IC, tx_cfg);
      delay(5);
      while (input != 'm')
      {
        if (Serial.available() > 0)
        {
          input = read_char();
        }
        LTC6804_adax();
        delay(10);
        error = LTC6804_rdaux(0, TOTAL_IC, aux_codes); // Set to read back all aux registers
        if (error == -1)
        {
          Serial.println("A PEC error was detected in the received data");
        }
        Serial.println("Reading back newly measured values:");
        print_aux();
        // Clear the registers and read back
        delay(5);
        LTC6804_clraux();
        delay(10);
        error = LTC6804_rdaux(0, TOTAL_IC, aux_codes); // Set to read back all aux registers
        if (error == -1)
        {
          Serial.println("A PEC error was detected in the received data");
        }
        Serial.println("Reading back cleared registers:");
        print_aux();
        Serial.println("--------------");
        delay(1000);
      }
      print_menu();
      break;

    case 10:
      Serial.println("transmit 'm' to quit");
      wakeup_sleep();
      LTC6804_wrcfg(TOTAL_IC, tx_cfg);
      while (input != 'm')
      {
        if (Serial.available() > 0)
        {
          input = read_char();
        }
        for (int ic = 0; ic < TOTAL_IC; ic++) { // Turn off discharging on previous cell
          if (balance_cell_index < 8) {
            tx_cfg[ic][4] = tx_cfg[ic][4] & ~(0b1 << balance_cell_index );
          } else {
            tx_cfg[ic][5] = tx_cfg[ic][5] & ~(0b1 << (balance_cell_index - 8));
          }
        }
        balance_cell_index = (balance_cell_index + 1) % 12;
        for (int ic = 0; ic < TOTAL_IC; ic++) { // Turn on discharging on next cell
          if (balance_cell_index < 8) {
            tx_cfg[ic][4] = tx_cfg[ic][4] | (0b1 << balance_cell_index);
          } else {
            tx_cfg[ic][5] = tx_cfg[ic][5] | (0b1 << (balance_cell_index - 8));
          }
        }
        wakeup_sleep();
        LTC6804_wrcfg(TOTAL_IC, tx_cfg);
        Serial.print("Balancing cell ");
        Serial.print(balance_cell_index);
        Serial.println(" for all ICs");
        delay(500);
      }
      for (int ic = 0; ic < TOTAL_IC; ic++) {
        tx_cfg[ic][4] = 0;
        tx_cfg[ic][5] = 0;
      }
      wakeup_sleep();
      LTC6804_wrcfg(TOTAL_IC, tx_cfg);
      print_menu();
      break;

    case 11:
      Serial.println("transmit 'm' to quit");
      wakeup_sleep();
      LTC6804_wrcfg(TOTAL_IC, tx_cfg);

      while (input != 'm')
      {
        if (Serial.available() > 0)
        {
          input = read_char();
        }
        
        if (balance_cell_index < 8) {
          tx_cfg[IC_index][4] = tx_cfg[IC_index][4] & ~(0b1 << balance_cell_index );
        } else {
          tx_cfg[IC_index][5] = tx_cfg[IC_index][5] & ~(0b1 << (balance_cell_index - 8));
        }

        balance_cell_index = (balance_cell_index + 1);
        if (balance_cell_index == 12) {
          IC_index = (IC_index + 1) % TOTAL_IC;
        }
        balance_cell_index %= 12;

        if (balance_cell_index < 8) {
          tx_cfg[IC_index][4] = tx_cfg[IC_index][4] | (0b1 << balance_cell_index);
        } else {
          tx_cfg[IC_index][5] = tx_cfg[IC_index][5] | (0b1 << (balance_cell_index - 8));
        }

        wakeup_sleep();
        LTC6804_wrcfg(TOTAL_IC, tx_cfg);

        Serial.print("Balancing cell ");
        Serial.print(balance_cell_index);
        Serial.print(" for IC ");
        Serial.println(IC_index);
        delay(500);
      }

      for (int ic = 0; ic < TOTAL_IC; ic++) {
        tx_cfg[ic][4] = 0;
        tx_cfg[ic][5] = 0;
      }

      wakeup_sleep();
      LTC6804_wrcfg(TOTAL_IC, tx_cfg);
      print_menu();
      break;

    default:
      Serial.println("Incorrect Option");
      break;
  }
}

/*!***********************************
  \brief Initializes the configuration array
 **************************************/
void init_cfg()
{
  for (int i = 0; i < TOTAL_IC; i++)
  {
    tx_cfg[i][0] = 0xFE;
    tx_cfg[i][1] = 0x00 ;
    tx_cfg[i][2] = 0x00 ;
    tx_cfg[i][3] = 0x00 ;
    tx_cfg[i][4] = 0x00 ;
    tx_cfg[i][5] = 0x00 ;
  }
}

/*!*********************************
  \brief Prints the main menu
***********************************/
void print_menu()
{
  Serial.println("Please enter LTC6804 Command");
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
      Serial.print(cell_codes[current_ic][i] * 0.0001, 4);
      Serial.print(",");
    }
    Serial.println();
  }
  Serial.println();
}

/*!****************************************************************************
  \brief Prints GPIO Voltage Codes and Vref2 Voltage Code onto the serial port
 *****************************************************************************/
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
      Serial.print(aux_codes[current_ic][i] * 0.0001, 4);
      Serial.print(",");
    }
    Serial.print(" Vref2");
    Serial.print(":");
    Serial.print(aux_codes[current_ic][5] * 0.0001, 4);
    Serial.println();
  }
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
    serial_print_hex(tx_cfg[current_ic][0]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][1]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][2]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][3]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][4]);
    Serial.print(", 0x");
    serial_print_hex(tx_cfg[current_ic][5]);
    Serial.print(", Calculated PEC: 0x");
    cfg_pec = pec15_calc(6, &tx_cfg[current_ic][0]);
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
    serial_print_hex(rx_cfg[current_ic][0]);
    Serial.print(", 0x");
    serial_print_hex(rx_cfg[current_ic][1]);
    Serial.print(", 0x");
    serial_print_hex(rx_cfg[current_ic][2]);
    Serial.print(", 0x");
    serial_print_hex(rx_cfg[current_ic][3]);
    Serial.print(", 0x");
    serial_print_hex(rx_cfg[current_ic][4]);
    Serial.print(", 0x");
    serial_print_hex(rx_cfg[current_ic][5]);
    Serial.print(", Received PEC: 0x");
    serial_print_hex(rx_cfg[current_ic][6]);
    Serial.print(", 0x");
    serial_print_hex(rx_cfg[current_ic][7]);
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
