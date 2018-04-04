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
}

void mc_fault_info::set_run_fault(int run) {
    int run_lo = run & 0xFFFF;
    int run_hi = (run >> 16) & 0xFFFF;
}

mc_fault_info::~mc_fault_info()
{
    delete ui;
}
