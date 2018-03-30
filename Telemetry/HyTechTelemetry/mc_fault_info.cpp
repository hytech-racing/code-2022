#include "mc_fault_info.h"
#include "ui_mc_fault_info.h"

mc_fault_info::mc_fault_info(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::mc_fault_info)
{
    ui->setupUi(this);
}

mc_fault_info::~mc_fault_info()
{
    delete ui;
}
