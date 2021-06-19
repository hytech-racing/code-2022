#ifndef XBEE_RCV_THREAD_H
#define XBEE_RCV_THREAD_H

#include "../../Libraries/HyTech17_Library/HyTech17.h"
#include "../../Libraries/XBTools/XBTools.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <termios.h>
#include <QThread>

class xbee_rcv_thread : public QThread {
    Q_OBJECT
public:
    xbee_rcv_thread();
    ~xbee_rcv_thread();
    void run();
    void exec();
    int configure_port(QString port);
    QByteArray xbee_port;
    QByteArray message;
    quint32 id;
    quint8 length;
    xbee_rcv_thread *thread;
signals:
    void updated(quint32, quint8, QByteArray);
    void xbee_error(QByteArray);
private:
    int xbee_device;
};

#endif // XBEE_RCV_THREAD_H
