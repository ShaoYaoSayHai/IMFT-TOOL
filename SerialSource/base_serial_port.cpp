#include "base_serial_port.h"
#include <QDebug>
#include <QThread>

BaseSerialPort::BaseSerialPort(QObject *parent) : QObject(parent)
{

}

BaseSerialPort::~BaseSerialPort()
{
    // 回收
    if( pxSerialPort != nullptr )
    {
        if( pxSerialPort->isOpen() )
        {
            pxSerialPort->close() ;
        }
        pxSerialPort->deleteLater() ;
        pxSerialPort = nullptr ;
    }
    qDebug()<<"释放BaseSerialPort"<<QThread::currentThreadId() ;
}

bool BaseSerialPort::getQSerialPortPtr()
{
    return pxSerialPort->isOpen() ;
}

void BaseSerialPort::SerialPortInit()
{
    pxSerialPort = new QSerialPort();
    // 槽函数::串口读取
    connect( pxSerialPort , &QSerialPort::readyRead , this , &BaseSerialPort::SerialPortReadAll );
    connect( pxSerialPort , &QSerialPort::errorOccurred , this , &BaseSerialPort::SerialPortErrorHappend );
}

/**
 * @brief BaseSerialPort::SerialPortConnect
 *        串口连接
 * @param portName 串口号
 * @param baudRate 波特率
 */
void BaseSerialPort::SerialPortConnect(QString portName, int baudRate)
{
    qDebug()<<"进入函数";
    // 已经连接直接返回
    if( bsp_connected )
    {
        emit ret_connect_status(ConnectStatus::Error) ;
        return ;
    }
    // 传入的端口名为空直接返回
    if( portName.isEmpty() ||( pxSerialPort == nullptr) )
    {
        emit ret_connect_status(ConnectStatus::Error) ;
        return ;
    }
    pxSerialPort->setPortName( portName );
    pxSerialPort->setBaudRate(baudRate) ;
//    pxSerialPort->setDataBits(QSerialPort::Data8);
//    pxSerialPort->setStopBits(QSerialPort::OneStop);
//    pxSerialPort->setParity(QSerialPort::NoParity);

    if( pxSerialPort->open(QIODevice::ReadWrite) )
    {
        bsp_connected = true ;// 设置已经连接成功
        emit ret_connect_status( ConnectStatus::Connected );
        qDebug()<<"连接成功";
        return ;
    }
    qDebug()<<"连接失败";
    emit ret_connect_status(ConnectStatus::Error) ;
}

void BaseSerialPort::SerialPortDisconnect()
{
    if (!pxSerialPort)
        return;

    if (pxSerialPort->isOpen()) {
        pxSerialPort->close();
    }
    bsp_connected = false;
    emit ret_connect_status(ConnectStatus::UnConnected);
    qDebug() << "串口断开" << QThread::currentThreadId();
}

//void BaseSerialPort::SerialPortWrite(QByteArray data)
//{
//    if( pxSerialPort )
//    {
//        pxSerialPort->write(data);
//    }
//}

void BaseSerialPort::SerialPortWrite(char *buffer, int len)
{
    if(pxSerialPort)
    {
        pxSerialPort->write(buffer , len) ;
    }
}

void BaseSerialPort::SerialPortReadAll()
{
    if( pxSerialPort->bytesAvailable() <= 0  )
    {
        return ;
    }
    QByteArray data = pxSerialPort->readAll() ;
    emit ret_read_data(data) ;
}

void BaseSerialPort::SerialPortErrorHappend(QSerialPort::SerialPortError error)
{
    qDebug()<<error ;
}






