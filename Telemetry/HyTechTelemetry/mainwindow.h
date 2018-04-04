#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include "../../Libraries/HyTech17_Library/HyTech17.h"
#include "../../Libraries/XBTools/XBTools.h"
#include "xbee_rcv_thread.h"
#include "mc_fault_info.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void on_update(quint32 id, quint8 length, QByteArray msg);
    void on_error(QByteArray err_str);

private slots:
    void on_mc_fault_button_clicked();

private:
    QPushButton *mc_fault_button;
    mc_fault_info *mcfw;
    void set_label(QLabel *label, int status, QString text);
    QString vsm_state_to_string(quint8 vsm);
    QString inverter_state_to_string(quint8 inv);
    QString discharge_state_to_string(quint8 discharge);
    Ui::MainWindow *ui;
    xbee_rcv_thread worker_thread;
    int mc_post_fault;
    int mc_run_fault;
};

#endif // MAINWINDOW_H
