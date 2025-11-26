#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include <QThread>
#include "base_serial_port.h"
#include <QDebug>

class SerialWorker : public QObject
{
    Q_OBJECT
public:
    explicit SerialWorker(QObject *parent = nullptr);
    ~SerialWorker();

    bool getCurrentSerialPortStatus();

private:
    QThread       m_thread;
    BaseSerialPort *m_serial = nullptr;
public slots:

    void startWorker();  // 创建 BaseSerialPort, 放到子线程, 调用 SerialPortInit()
    void stopWorker();   // 停止线程，回收资源

    void openPort(QString portName, int baudRate);
    void closePort();
    void writeData(const QByteArray &data);

private slots:
    // 仅在子线程执行的槽
    void onThreadStarted();
    void onThreadFinished();

signals:

    // 对外转发 BaseSerialPort 的信号
    void sig_connectStatus(int status);
    void sig_readData(const QByteArray &data);

};

#endif // SERIALWORKER_H
