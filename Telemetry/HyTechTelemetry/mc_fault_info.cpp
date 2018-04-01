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
}

void mc_fault_info::set_run_fault(int run) {
    int run_lo = run & 0xFFFF;
    int run_hi = (run >> 16) & 0xFFFF;
}

mc_fault_info::~mc_fault_info()
{
    delete ui;
}
