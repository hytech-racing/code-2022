#include "LTC6811_2.h"
#include <string.h>
#include <stdio.h>

// debug mode
#define DEBUG false
// SPI slave select pin, as required for Teensy 4.0
#define SS 10
// SPI alternate pin definitions (not needed unless using Teensy 3.2 alternate SPI pins)
#define MOSI 7 // pin 11 by default
#define MISO 8 // pin 12 by default
#define SCLK 14 // pin 13 by default

/* Note; SPI transfer is a simultaneous send/receive; the spi_write_reg function disregards the received data, and
 * the spi_read_reg function sends dummy values in order to read in values from the slave device. */
// SPI write
void LTC6811_2::spi_write_reg(uint8_t *cmd, uint8_t *cmd_pec, uint8_t *data, uint8_t *data_pec) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    delayMicroseconds(1);
    SPI.transfer(cmd[0]);
    SPI.transfer(cmd[1]);
    SPI.transfer(cmd_pec[0]);
    SPI.transfer(cmd_pec[1]);
    for (int i = 0; i < 6; i++) {
        SPI.transfer(data[i]);
    }
    SPI.transfer(data_pec[0]);
    SPI.transfer(data_pec[1]);
    digitalWrite(SS, HIGH);
    delayMicroseconds(1);
    SPI.endTransaction();
#if DEBUG
    //Serial.println("SPI write complete.");
#endif
}
// SPI read; IF CODE DOES NOT WORK, THIS IS A GOOD PLACE TO START DEBUGGING
void LTC6811_2::spi_read_reg(uint8_t *cmd, uint8_t* cmd_pec, uint8_t *data_in) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    delayMicroseconds(1);
    SPI.transfer(cmd[0]);
    SPI.transfer(cmd[1]);
    SPI.transfer(cmd_pec[0]);
    SPI.transfer(cmd_pec[1]);
    // read in data and PEC; bytes 0 to 5 data bytes; bytes 6 to 7 PEC bytes
    for (int i = 0; i < 8; i++) {
        data_in[i] = SPI.transfer(0); // transfer dummy value over SPI in order to read bytes into data
#if DEBUG
        Serial.print("SPI in byte: "); Serial.println(data_in[i], BIN);
#endif
    }
    digitalWrite(SS, HIGH);
    delayMicroseconds(1);
    SPI.endTransaction();
}

// SPI command
void LTC6811_2::spi_cmd(uint8_t *cmd, uint8_t* cmd_pec) {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    delayMicroseconds(1);
    SPI.transfer(cmd[0]);
    SPI.transfer(cmd[1]);
    SPI.transfer(cmd_pec[0]);
    SPI.transfer(cmd_pec[1]);
    digitalWrite(SS, HIGH);
    delayMicroseconds(1);
    SPI.endTransaction();
}

// returns the address of the specific LTC6811-2 chip to send command to
uint8_t LTC6811_2::get_cmd_address() {
    return 0x80 | (this->address << 3);
}

/* Packet Error Code (PEC) generated using algorithm specified on datasheet page 53-54
 * PEC array structure:
 * pec[0] = bits 14 downto 7 of PEC
 * pec[1] = bits 6 downto 0 of PEC; LSB of pec[1] is a padded zero as per datasheet
 */
 void LTC6811_2::init_PEC15_Table() {
     for (int i = 0; i < 256; i++) {
         uint16_t remainder   = i << 7;
         for (int bit = 8; bit > 0; --bit) {
             if(remainder & 0x4000) {
                 remainder = ((remainder << 1));
                 remainder = (remainder ^ CRC15_POLY);
             } else {
                 remainder = ((remainder << 1));
             }
         }
         pec15Table_pointer[i] = remainder&0xFFFF;
     }
 }

//PEC lookup function
void LTC6811_2::generate_pec(uint8_t *data, uint8_t *pec, int num_bytes) {
    uint16_t remainder;
    uint16_t addr;
    remainder = 16; //PEC seed
    for (int i = 0; i < num_bytes; i++) {
        addr = ((remainder >> 7) ^ data[i]) & 0xff; //calculate PEC table address
        remainder = (remainder << 8 ) ^ pec15Table_pointer[addr];
    }
    remainder = remainder * 2; //The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
    pec[0] = (uint8_t) ((remainder >> 8) & 0xFF);
    pec[1] = (uint8_t) (remainder & 0xFF);
}
// setter for PEC error flag
void LTC6811_2::set_pec_error(bool flag) {
    pec_error = flag;
}
// getter for PEC error flag
bool LTC6811_2::get_pec_error() {
    return pec_error;
}
// Set the ADC mode per datasheet page 22; NOTE: ADCOPT is updated every time rdcfga is called
void LTC6811_2::set_adc_mode(ADC_MODE mode) {
    // NOTE: FAST, NORMAL, and FILTERED not well defined for ADCOPT; see datasheet page 61
    adc_mode = static_cast<uint8_t>(mode);
}
/* Set the delay needed for ADC operations to complete. NOTE: delays for the longer of the two possible times to
 * accommodate either ADCOPT = 0 or ADCOPT = 1 */
const void LTC6811_2::adc_delay() {
     switch(adc_mode) {
         case 0:
             delay(13);
             break;
         case 1:
             delayMicroseconds(1300);
             break;
         case 2:
             delayMicroseconds(3100);
         case 3:
             //Serial.println("begin delay 203 ms");
             delay(203);
             //Serial.println("end delay 203 ms");
     }

 }
// Set the discharge permission during cell measurement, see datasheet page 73
void LTC6811_2::set_discharge_permit(DISCHARGE permit) {
    discharge_permitted = static_cast<uint8_t>(permit);
}

// Write register commands
// General write register group handler
void LTC6811_2::write_register_group(uint16_t cmd_code, const uint8_t *buffer) {
    /* cmd array structure as related to datasheet:
     * cmd[0] = CMD0;
     * cmd[1] = CMD1;
     */
    uint8_t cmd[2] = {(uint8_t) (get_cmd_address() | cmd_code >> 8), (uint8_t) cmd_code};
#if DEBUG
    uint16_t dec_cmd_code = cmd[0] << 8 | cmd[1];
    Serial.print("Raw Command Code: ");
    Serial.println(cmd_code, BIN);
    Serial.print("Addressed Command Code: ");
    Serial.println(dec_cmd_code, BIN);
#endif
    uint8_t cmd_pec[2];
    uint8_t data[6];
    uint8_t data_pec[2];
    // generate PEC from command bytes
    generate_pec(cmd, cmd_pec, 2);
    for (int i = 0; i < 6; i++) {
        data[i] = buffer[i];
    }
    // generate PEC from data bytes
    generate_pec(data, data_pec, 6);
#if DEBUG
    uint16_t dec_pec = cmd_pec[0] << 8 | cmd_pec[1];
    Serial.print("Command PEC: ");
    Serial.println(dec_pec, BIN);
    Serial.println("Prepare to spi_write_reg");
#endif
    // write out via SPI
    spi_write_reg(cmd, cmd_pec, data, data_pec);
}
// Write Configuration Register Group A
void LTC6811_2::wrcfga(Reg_Group_Config reg_group) {
    write_register_group(0x1, reg_group.buf());
}
// Write S Control Register Group
void LTC6811_2::wrsctrl(Reg_Group_S_Ctrl reg_group) {
    write_register_group(0x14, reg_group.buf());
}
// Write PWM Register group
void LTC6811_2::wrpwm(Reg_Group_PWM reg_group) {
    write_register_group(0x20, reg_group.buf());
}
// Write COMM Register Group
void LTC6811_2::wrcomm(Reg_Group_COMM reg_group) {
    write_register_group(0x721, reg_group.buf());
}


// Read register commands
// general read register group handler
void LTC6811_2::read_register_group(uint16_t cmd_code, uint8_t *data) {
    // bytes 0 to 5 data bytes; bytes 6 to 7 PEC bytes
    uint8_t data_in[8];
    uint8_t cmd[2] = {(uint8_t) (get_cmd_address() | cmd_code >> 8), (uint8_t) cmd_code};
    uint8_t cmd_pec[2];
    uint8_t data_pec[2];
    // Generate PEC from command bytes
    generate_pec(cmd, cmd_pec, 2);
    // read in via SPI
    spi_read_reg(cmd, cmd_pec, data_in);
    // generate PEC from read-in data bytes
    generate_pec(data_in, data_pec, 6);
    // Check if the PEC locally generated on the data that is read in matches the PEC that is read in
    if ((data_pec[0] == data_in[6]) || (data_pec[1] == data_in[7])) {
        // After confirming matching PECs, add the data that was read in to the array that was passed into the function
        for (int i = 0; i < 6; i++) {
            data[i] = data_in[i];
        }
        pec_error = false;
#if DEBUG
        Serial.println("PEC OK");
#endif
    } else {
        // set flag indicating there is a PEC error
        for (int i = 0; i < 6; i++) {
            data[i] = data_in[i];
        }
        pec_error = true;
#if DEBUG
        Serial.println("PEC Error");
        Serial.print("LTC PEC: "); Serial.print(data_in[6], BIN); Serial.println(data_in[7], BIN);
        Serial.print("Calculated PEC: "); Serial.print(data_pec[0], BIN); Serial.println(data_pec[1], BIN);
#endif
    }
}
// Read Configuration Register Group A
Reg_Group_Config LTC6811_2::rdcfga() {
    uint8_t buffer[6];
    // If PECs do not match, return empty buffer
    read_register_group(0x2, buffer);
    return {buffer};
}
// Read Cell Voltage Register Group A
Reg_Group_Cell_A LTC6811_2::rdcva() {
    uint8_t buffer[6];
    read_register_group(0x4, buffer);
    return {buffer};
}
// Read Cell Voltage Register Group B
Reg_Group_Cell_B LTC6811_2::rdcvb() {
    uint8_t buffer[6];
    read_register_group(0x6, buffer);
    return {buffer};
}
// Read Cell Voltage Register Group C
Reg_Group_Cell_C LTC6811_2::rdcvc() {
    uint8_t buffer[6];
    read_register_group(0x8, buffer);
    return {buffer};
}
// Read Cell Voltage Register Group D
Reg_Group_Cell_D LTC6811_2::rdcvd() {
    uint8_t buffer[6];
    read_register_group(0xA, buffer);
    return {buffer};
}
// Read Auxiliary Register Group A
Reg_Group_Aux_A LTC6811_2::rdauxa() {
    uint8_t buffer[6];
    read_register_group(0xC, buffer);
    return {buffer};

}
// Read Auxiliary Register Group B
Reg_Group_Aux_B LTC6811_2::rdauxb() {
    uint8_t buffer[6];
    read_register_group(0xE, buffer);
    return {buffer};
}
// Read Status Register Group A
Reg_Group_Status_A LTC6811_2::rdstata() {
    uint8_t buffer[6];
    read_register_group(0x10, buffer);
    return {buffer};
}
// Read Status Register Group B
Reg_Group_Status_B LTC6811_2::rdstatb() {
    uint8_t buffer[6];
    read_register_group(0x12, buffer);
    return {buffer};
}
// Read S Control Register Group
Reg_Group_S_Ctrl LTC6811_2::rdsctrl() {
    uint8_t buffer[6];
    read_register_group(0x16, buffer);
    return {buffer};
}
// Read PWM Register Group
Reg_Group_PWM LTC6811_2::rdpwm() {
    uint8_t buffer[6];
    read_register_group(0x22, buffer);
    return {buffer};
}
// Read COMM Register Group
Reg_Group_COMM LTC6811_2::rdcomm() {
    uint8_t buffer[6];
    read_register_group(0x722, buffer);
    return {buffer};
}

// General non-register command handler
void LTC6811_2::non_register_cmd(uint16_t cmd_code, bool delay_enable) {
    uint8_t cmd[2] = {(uint8_t) (get_cmd_address() | cmd_code >> 8), (uint8_t) cmd_code};
    uint8_t cmd_pec[2];
    // generate PEC from command bytes
    generate_pec(cmd, cmd_pec, 2);
    // write out via SPI
    spi_cmd(cmd, cmd_pec);
    if(delay_enable)
    {
      adc_delay();
    }
}

// Start -action- commands
// Start S Control Pulsing and Poll Status
void LTC6811_2::stsctrl() {
    non_register_cmd(0x19, false);
}
// Start Cell Voltage ADC Conversion and Poll Status
void LTC6811_2::adcv(CELL_SELECT cell_select, bool delay) {
    uint16_t adc_cmd = 0x260 | (adc_mode << 7) | (discharge_permitted << 4) | static_cast<uint8_t>(cell_select);
    non_register_cmd(adc_cmd, delay);
}
// Start GPIOs ADC Conversion and Poll Status
void LTC6811_2::adax(GPIO_SELECT gpio_select, bool delay) {
    uint16_t adc_cmd = 0x460 | (adc_mode << 7) | static_cast<uint8_t>(gpio_select);
    non_register_cmd(adc_cmd, delay);
}
// Start Combined Cell Voltage and GPIO1, GPIO2 Conversion and Poll Status
void LTC6811_2::adcvax(bool delay) {
    uint16_t adc_cmd = 0x46F | (adc_mode << 7) | (discharge_permitted << 4);
    non_register_cmd(adc_cmd, delay);
}
// Start Combined Cell Voltage and SC Conversion and Poll Status
void LTC6811_2::adcvsc(bool delay) {
    uint16_t adc_cmd = 0x467 | (adc_mode << 7) | (discharge_permitted << 4);
    non_register_cmd(adc_cmd, delay);
}

// Clear register commands
// Clear Cell Voltage Register Groups
void LTC6811_2::clrsctrl() {
    non_register_cmd(0x18, false);
}
// Clear Auxiliary Register Group
void LTC6811_2::clraux() {
    non_register_cmd(0x712, false);
}
// Clear Status Register Groups
void LTC6811_2::clrstat() {
    non_register_cmd(0x713, false);
}
// Poll ADC Conversion Status
void LTC6811_2::pladc() {
    // NOTE: in parallel isoSPI mode, this command is not necessarily needed; see datasheet page 55
    non_register_cmd(0x714, false);
}
// Diagnose MUX and Poll Status: sets MUXFAIL bit to 1 in Status Register Group B if any channel decoder fails; see datasheet pg. 32
void LTC6811_2::diagn() {
    non_register_cmd(0x715, false);
}
// Start I2C/SPI Communication to a slave device when the LTC6811-2 acts as the master
void LTC6811_2::stcomm() {
    non_register_cmd(0x723, false);
}

/* Wakeup functions
 * The LTC6811 wakes up after receiving a differential pulse on the isoSPI port.
 * This can be achieved by toggling the chip select signal; see LTC6820 datasheet page 11.
 */
// Wakeup LTC6811 from core SLEEP state and/ or isoSPI IDLE state to ready for ADC measurements or isoSPI comms
void LTC6811_2::wakeup() {
    digitalWrite(SS, LOW);
    delayMicroseconds(1);
    SPI.transfer(0);
    digitalWrite(SS, HIGH);
    delayMicroseconds(400); //t_wake is 400 microseconds; wait that long to ensure device has turned on.
}
