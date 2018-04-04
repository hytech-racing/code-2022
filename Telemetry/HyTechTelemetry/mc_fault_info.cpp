#include "mc_fault_info.h"
#include "ui_mc_fault_info.h"

mc_fault_info::mc_fault_info(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mc_fault_info)
{
    ui->setupUi(this);
}

void mc_fault_info::set_post_fault(int post) {
    int post_lo = post & 0xFFFF;
    int post_hi = (post >> 16) & 0xFFFF;
    if (post_lo & 0x0001)
        ui->post_text->appendPlainText("HW Gate Desaturation Fault\n");
    if ((post_lo & 0x0002) >> 1)
        ui->post_text->appendPlainText("HW Overcurrent Fault\n");
    if ((post_lo & 0x0004) >> 2)
        ui->post_text->appendPlainText("Accelerator Shorted\n");
    if ((post_lo & 0x0008) >> 3)
        ui->post_text->appendPlainText("Accelerator Open\n");
    if ((post_lo & 0x0010) >> 4)
        ui->post_text->appendPlainText("Current Sensor Low\n");
    if ((post_lo & 0x0020) >> 5)
        ui->post_text->appendPlainText("Current Sensor High\n");
    if ((post_lo & 0x0040) >> 6)
        ui->post_text->appendPlainText("Module Temperature Low\n");
    if ((post_lo & 0x0080) >> 7)
        ui->post_text->appendPlainText("Module Temperature High\n");
    if ((post_lo & 0x0100) >> 8)
        ui->post_text->appendPlainText("PCB Temperature Low\n");
    if ((post_lo & 0x0200) >> 9)
        ui->post_text->appendPlainText("PCB Temperature High\n");
    if ((post_lo & 0x0400) >> 10)
        ui->post_text->appendPlainText("Gate Drive PCB Temp Low\n");
    if ((post_lo & 0x0800) >> 11)
        ui->post_text->appendPlainText("Gate Drive PCB Temp High\n");
    if ((post_lo & 0x1000) >> 12)
        ui->post_text->appendPlainText("5v Sense Voltage Low\n");
    if ((post_lo & 0x2000) >> 13)
        ui->post_text->appendPlainText("5v Sense Voltage High\n");
    if ((post_lo & 0x4000) >> 14)
        ui->post_text->appendPlainText("12v Sense Voltage Low\n");
    if ((post_lo & 0x8000) >> 15)
        ui->post_text->appendPlainText("12 Sense Voltage High\n");
    if (post_hi & 0x0001)
        ui->post_text->appendPlainText("25v Sense Voltage Low\n");
    if ((post_hi & 0x0002) >> 1)
        ui->post_text->appendPlainText("25v Sense Voltage High\n");
    if ((post_hi & 0x0004) >> 2)
        ui->post_text->appendPlainText("15v Sense Voltage Low\n");
    if ((post_hi & 0x0008) >> 3)
        ui->post_text->appendPlainText("15v Sense Voltage High\n");
    if ((post_hi & 0x0010) >> 4)
        ui->post_text->appendPlainText("DC Bus Voltage High\n");
    if ((post_hi & 0x0020) >> 5)
        ui->post_text->appendPlainText("DC Bus Voltage Low\n");
    if ((post_hi & 0x0040) >> 6)
        ui->post_text->appendPlainText("Precharge Timeout\n");
    if ((post_hi & 0x0080) >> 7)
        ui->post_text->appendPlainText("Precharge Voltage Fail\n");
    if ((post_hi & 0x0100) >> 8)
        ui->post_text->appendPlainText("EEPROM Checksum Invalid\n");
    if ((post_hi & 0x0200) >> 9)
        ui->post_text->appendPlainText("EEPROM Data Out of Range\n");
    if ((post_hi & 0x0400) >> 10)
        ui->post_text->appendPlainText("EEPROM Update Required\n");
    if ((post_hi & 0x0800) >> 11)
        ui->post_text->appendPlainText("Reserved 1\n");
    if ((post_hi & 0x1000) >> 12)
        ui->post_text->appendPlainText("Reserved 2\n");
    if ((post_hi & 0x2000) >> 13)
        ui->post_text->appendPlainText("Reserved 3\n");
    if ((post_hi & 0x4000) >> 14)
        ui->post_text->appendPlainText("Brake Shorted\n");
    if ((post_hi & 0x8000) >> 15)
        ui->post_text->appendPlainText("Brake Open\n");
}

void mc_fault_info::set_run_fault(int run) {
    int run_lo = run & 0xFFFF;
    int run_hi = (run >> 16) & 0xFFFF;
    if (run_lo & 0x0001)
        ui->run_text->appendPlainText("Motor Overspeed\n");
    if ((run_lo & 0x0002) >> 1)
        ui->run_text->appendPlainText("Overcurrent\n");
    if ((run_lo & 0x0004) >> 2)
        ui->run_text->appendPlainText("Overvoltage\n");
    if ((run_lo & 0x0008) >> 3)
        ui->run_text->appendPlainText("Inverter Overtemp\n");
    if ((run_lo & 0x0010) >> 4)
        ui->run_text->appendPlainText("Accel Input Shorted\n");
    if ((run_lo & 0x0020) >> 5)
        ui->run_text->appendPlainText("Accel Input Open\n");
    if ((run_lo & 0x0040) >> 6)
        ui->run_text->appendPlainText("Direction Command Fault\n");
    if ((run_lo & 0x0080) >> 7)
        ui->run_text->appendPlainText("Inverter Response Timeout\n");
    if ((run_lo & 0x0100) >> 8)
        ui->run_text->appendPlainText("HW Gate/Desaturation Fault\n");
    if ((run_lo & 0x0200) >> 9)
        ui->run_text->appendPlainText("HW Overcurrent Fault\n");
    if ((run_lo & 0x0400) >> 10)
        ui->run_text->appendPlainText("Undervoltage\n");
    if ((run_lo & 0x0800) >> 11)
        ui->run_text->appendPlainText("CAN Command Msg Lost\n");
    if ((run_lo & 0x1000) >> 12)
        ui->run_text->appendPlainText("Motor Overtemperature\n");
    if ((run_lo & 0x2000) >> 13)
        ui->run_text->appendPlainText("Reserved 1\n");
    if ((run_lo & 0x4000) >> 14)
        ui->run_text->appendPlainText("Reserved 2\n");
    if ((run_lo & 0x8000) >> 15)
        ui->run_text->appendPlainText("Reserved 3\n");
    if (run_hi & 0x0001)
        ui->run_text->appendPlainText("Brake Input Shorted\n");
    if ((run_hi & 0x0002) >> 1)
        ui->run_text->appendPlainText("Brake Input Open\n");
    if ((run_hi & 0x0004) >> 2)
        ui->run_text->appendPlainText("Module A Overtemp\n");
    if ((run_hi & 0x0008) >> 3)
        ui->run_text->appendPlainText("Module B Overtemp\n");
    if ((run_hi & 0x0010) >> 4)
        ui->run_text->appendPlainText("Module C Overtemp\n");
    if ((run_hi & 0x0020) >> 5)
        ui->run_text->appendPlainText("PCB Overtemp\n");
    if ((run_hi & 0x0040) >> 6)
        ui->run_text->appendPlainText("Gate Drive 1 Overtemp\n");
    if ((run_hi & 0x0080) >> 7)
        ui->run_text->appendPlainText("Gate Drive 2 Overtemp\n");
    if ((run_hi & 0x0100) >> 8)
        ui->run_text->appendPlainText("Gate Drive 3 Overtemp\n");
    if ((run_hi & 0x0200) >> 9)
        ui->run_text->appendPlainText("Current Sensor Fault\n");
    if ((run_hi & 0x0400) >> 10)
        ui->run_text->appendPlainText("Reserved 4\n");
    if ((run_hi & 0x0800) >> 11)
        ui->run_text->appendPlainText("Reserved 5\n");
    if ((run_hi & 0x1000) >> 12)
        ui->run_text->appendPlainText("Reserved 6\n");
    if ((run_hi & 0x2000) >> 13)
        ui->run_text->appendPlainText("Reserved 7\n");
    if ((run_hi & 0x4000) >> 14)
        ui->run_text->appendPlainText("Resolver Not Connected\n");
    if ((run_hi & 0x8000) >> 15)
        ui->run_text->appendPlainText("Inverter Discharge Active\n");
}

mc_fault_info::~mc_fault_info()
{
    delete ui;
}
