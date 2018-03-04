#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../../Libraries/HyTech17_Library/HyTech17.h"
#include "../../Libraries/XBTools/XBTools.h"

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

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
