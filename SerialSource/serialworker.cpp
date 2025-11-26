#include "serialworker.h"
#include <QDebug>


SerialWorker::SerialWorker(QObject *parent) : QObject(parent)
{
    // 线程启动/结束的槽
    connect(&m_thread, &QThread::started,
            this, &SerialWorker::onThreadStarted);
    connect(&m_thread, &QThread::finished,
            this, &SerialWorker::onThreadFinished);
}

SerialWorker::~SerialWorker()
{
    qDebug()<<"SerialWorker delete start" ;
    this->stopWorker() ;
    qDebug()<<"SerialWorker delete success" ;
}

bool SerialWorker::getCurrentSerialPortStatus()
{
    bool status = false ;
    status = m_serial->getQSerialPortPtr();
    return status ;
}

void SerialWorker::startWorker()
{
    if( m_thread.isRunning() )
    {
        return ;
    }

    m_serial = new BaseSerialPort();
    m_serial->moveToThread(&m_thread) ;

    // 将 BaseSerialPort 的信号转发到本类（本类在主线程中）
    connect(m_serial, &BaseSerialPort::ret_connect_status,
            this, &SerialWorker::sig_connectStatus);
    connect(m_serial, &BaseSerialPort::ret_read_data,
            this, &SerialWorker::sig_readData);

    // 线程结束时，自动销毁 m_serial（在子线程中调用 deleteLater）
    connect(&m_thread, &QThread::finished,
            m_serial, &QObject::deleteLater);

    // 等线程启动后，在子线程中完成 SerialPortInit
    connect(&m_thread, &QThread::started,
            m_serial, &BaseSerialPort::SerialPortInit,
            Qt::QueuedConnection);

    // 启动线程
    m_thread.start();
}

void SerialWorker::stopWorker()
{
    if (!m_thread.isRunning())
        return;

    // 1. 通知 BaseSerialPort 断开串口（在子线程中执行）
    if (m_serial) {
        QMetaObject::invokeMethod(m_serial, "SerialPortDisconnect",
                                  Qt::QueuedConnection);
    }

    // 2. 请求线程结束
    m_thread.quit();

    // 3. 等待线程结束（阻塞当前线程，通常是 UI 线程，时间应该很短）
    m_thread.wait();

    // 线程结束后 onThreadFinished 会被调用，m_serial 会被 deleteLater
    m_serial = nullptr;
}

void SerialWorker::openPort(QString portName, int baudRate)
{
    if (!m_thread.isRunning() || !m_serial) {
        qWarning() << "SerialWorker: thread not running or serial not created";
        return;
    }

    // 通过 invokeMethod 以 Queued 方式调用子线程对象的槽
    QMetaObject::invokeMethod(m_serial, "SerialPortConnect",
                              Qt::QueuedConnection,
                              Q_ARG(QString, portName),
                              Q_ARG(int, baudRate));
}

void SerialWorker::closePort()
{
    if (!m_thread.isRunning() || !m_serial)
        return;

    QMetaObject::invokeMethod(m_serial, "SerialPortDisconnect",
                              Qt::QueuedConnection);
}

void SerialWorker::writeData(const QByteArray &data)
{
    if (!m_thread.isRunning() || !m_serial)
        return;

    // 因为 BaseSerialPort::SerialPortWrite 接口是 char* + len
    // 这里用 lambda 包装一下，或者改 BaseSerialPort 的接口。
    QByteArray copy = data;  // 注意：要拷贝一份，保证在子线程执行时数据依然有效

    QMetaObject::invokeMethod(m_serial, [this, copy]() {
        // 这个 lambda 在子线程中执行
        if (!m_serial)
            return;
        m_serial->SerialPortWrite(const_cast<char*>(copy.data()), copy.size());
    }, Qt::QueuedConnection);
}

void SerialWorker::onThreadStarted()
{
    qDebug() << "SerialWorker: thread started, id =" << m_thread.currentThreadId();
}

void SerialWorker::onThreadFinished()
{
    qDebug() << "SerialWorker: thread finished";
}
