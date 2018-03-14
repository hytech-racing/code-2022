#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Show input dialog to choose serial port
    bool ok;
    QString serial_port = QInputDialog::getText(this, tr("Serial Configuration"),
                                                tr("Enter serial port:"), QLineEdit::Normal,
                                                tr("/dev/cu."), &ok);
    if (ok && !serial_port.isEmpty()) {
        worker_thread.configure_port(serial_port);
        worker_thread.start();
        connect(&worker_thread, &xbee_rcv_thread::updated,
                this, &MainWindow::on_update);
        connect(&worker_thread, &xbee_rcv_thread::xbee_error,
                this, &MainWindow::on_error);
    }
}

MainWindow::~MainWindow()
{
    worker_thread.quit();
    delete ui;
}

void MainWindow::on_update(quint32 id, quint8 length, QByteArray msg) {
    if (id == ID_RCU_STATUS) {
        RCU_status rcus((unsigned char*) msg.data());
        ui->glv_voltage->display(rcus.get_glv_battery_voltage() / 10.0);
        ui->rcu_temperature->display(rcus.get_temperature() / 100.0);
    }
    if (id == ID_FCU_STATUS) {
        FCU_status fcus((unsigned char*) msg.data());
        if (fcus.get_accelerator_implausibility())
            set_label(ui->accel_implausibility, 1, "BAD");
        else
            set_label(ui->accel_implausibility, 0, "OK");

        if (fcus.get_brake_implausibility())
            set_label(ui->brake_implausibility, 1, "BAD");
        else
            set_label(ui->brake_implausibility, 0, "OK");
    }
    if (id == ID_BMS_STATUS) {
        BMS_status bmss((unsigned char*) msg.data());
        if (bmss.get_overvoltage())
            set_label(ui->cell_overvoltage, 1, "BAD");
        else
            set_label(ui->cell_overvoltage, 0, "OK");

        if (bmss.get_undervoltage())
            set_label(ui->cell_undervoltage, 1, "BAD");
        else
            set_label(ui->cell_undervoltage, 0, "OK");

        if (bmss.get_total_voltage_high())
            set_label(ui->bms_total_voltage, 1, "BAD");
        else
            set_label(ui->bms_total_voltage, 0, "OK");

        if (bmss.get_charge_overcurrent() || bmss.get_discharge_overcurrent())
            set_label(ui->charge_current, 1, "OVER");
        else
            set_label(ui->charge_current, 0, "OK");

        if (bmss.get_charge_overtemp() || bmss.get_discharge_overtemp())
            set_label(ui->bms_temperature, 1, "OVER");
        else if (bmss.get_undertemp())
            set_label(ui->bms_temperature, 1, "UNDER");
        else
            set_label(ui->bms_temperature, 0, "OK");

        ui->current->display(bmss.get_current());   // FIND OUT FORMAT
    }
    if (id == ID_BMS_VOLTAGES) {
        BMS_voltages bmsv((unsigned char*) msg.data());
        ui->avg_voltage->display(bmsv.get_average() / 10.0);
        ui->low_voltage->display(bmsv.get_low() / 10.0);
        ui->high_voltage->display(bmsv.get_high() / 10.0);
        ui->total_voltage->display(bmsv.get_total() / 10.0);
    }
}

void MainWindow::set_label(QLabel *label, int status, QString text) {
    if (status == 0) { // normal
        label->setStyleSheet("QLabel {color: black;}");
    } else if (status == 1) { // error
        label->setStyleSheet("QLabel {color: red;}");
    }
    label->setText(text);
}

void MainWindow::on_error(QByteArray err_str) {
    QMessageBox::critical(this, tr("Error"), tr(err_str));
}
