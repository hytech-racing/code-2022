#pragma once
#include <string.h>
#include <stdint.h>
#include <SPI.h>
#include <Arduino.h>
#ifdef HT_DEBUG_EN
#endif

#include "Register Groups/Reg_Group_Aux_A.h"
#include "Register Groups/Reg_Group_Aux_B.h"
#include "Register Groups/Reg_Group_Cell_A.h"
#include "Register Groups/Reg_Group_Cell_B.h"
#include "Register Groups/Reg_Group_Cell_C.h"
#include "Register Groups/Reg_Group_Cell_D.h"
#include "Register Groups/Reg_Group_COMM.h"
#include "Register Groups/Reg_Group_Config.h"
#include "Register Groups/Reg_Group_PWM.h"
#include "Register Groups/Reg_Group_S_Ctrl.h"
#include "Register Groups/Reg_Group_Status_A.h"
#include "Register Groups/Reg_Group_Status_B.h"
#include "option_enums.h"

class LTC6811_2 {
public:
    //Variables for the PEC functions
    static uint16_t* pec15Table_pointer;
    static const uint16_t CRC15_POLY = 0x4599;

    LTC6811_2() = default;
    LTC6811_2(int addr_) :
            address(addr_),
            pec_error(false),
            adc_mode(0x3),
            discharge_permitted(0x0) { };
    void init();

    //write register commands
    void wrcfga(Reg_Group_Config reg_group);
    void wrsctrl(Reg_Group_S_Ctrl reg_group);
    void wrpwm(Reg_Group_PWM reg_group);
    void wrcomm(Reg_Group_COMM reg_group);


    //read register commands
    Reg_Group_Config rdcfga();

    Reg_Group_Cell_A rdcva();
    Reg_Group_Cell_B rdcvb();
    Reg_Group_Cell_C rdcvc();
    Reg_Group_Cell_D rdcvd();

    Reg_Group_Aux_A rdauxa();

    Reg_Group_Status_A rdstata();
    Reg_Group_Status_B rdstatb();

    Reg_Group_S_Ctrl rdsctrl();
    Reg_Group_PWM rdpwm();

    Reg_Group_COMM rdcomm();


    // start -action- commands
    void stsctrl();
    void adcv(CELL_SELECT cell_select);
    // void adow(ADC_MODE adc_mode, OPEN_WIRE_CURRENT_PUP pup, DISCHARGE discharge, CELL_SELECT cell_select)
    // void cvst(ADC_MODE adc_mode, SELF_TEST_MODE self_test);
    // void adol(ADC_MODE adc_mode, DISCHARGE discharge);
    void adax(GPIO_SELECT gpio_select);
    // void adaxd(ADC_MODE adc_mode, GPIO_SELECT gpio_select);
    // void axst(ADC_MODE adc_mode, SELF_TEST_MODE self_test);
    // void adstat(ADC_MODE adc_mode, STATUS_GROUP_SELECT status_group);
    // void adstatd(ADC_MODE adc_mode, STATUS_GROUP_SELECT status_group);
    // void statst(ADC_MODE adc_mode, SELF_TEST_MODE self_test);
    void adcvax();
    void adcvsc();


    // clear commands
    void clrsctrl();
    void clraux();
    void clrstat();

    //misc commands
    void pladc();
    void diagn();
    void stcomm();
    void wakeup();

    // handlers and helper functions
    void spi_write(uint8_t *cmd, uint8_t *cmd_pec, uint8_t *data, uint8_t *data_pec);
    void spi_read(uint8_t *cmd, uint8_t* cmd_pec, uint8_t *data_in);
    void spi_cmd(uint8_t *cmd, uint8_t* cmd_pec);
    void write_register_group(uint16_t cmd_code, const uint8_t *buffer);
    void read_register_group(uint16_t cmd_code, uint8_t *data);
    void non_register_cmd(uint16_t cmd_code);
    uint8_t get_cmd_address();
    static void init_PEC15_Table();
    void generate_pec(uint8_t *data, uint8_t *pec, int num_bytes);
    void set_pec_error(bool flag);
    bool get_pec_error();
    void set_adc_mode(ADC_MODE mode);
    void set_discharge_permit(DISCHARGE permit);

private:
    uint8_t address;
    bool pec_error;
    uint8_t  adc_mode;
    uint8_t discharge_permitted;
};