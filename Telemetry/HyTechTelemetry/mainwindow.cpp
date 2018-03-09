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
    if (id == ID_BMS_VOLTAGES) {
        BMS_voltages bmsv((unsigned char*) msg.data());
        ui->avg_voltage->display(bmsv.get_average() / 10.0);
        ui->low_voltage->display(bmsv.get_low() / 10.0);
        ui->high_voltage->display(bmsv.get_high() / 10.0);
        ui->total_voltage->display(bmsv.get_total() / 10.0);
    }
}

void MainWindow::on_error(QByteArray err_str) {
    QMessageBox::critical(this, tr("Error"), tr(err_str));
}
