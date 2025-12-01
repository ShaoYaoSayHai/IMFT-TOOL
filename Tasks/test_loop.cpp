#include "test_loop.h"
#include <QDebug>
#include <QThread>
#include "hexprintf.h"

#define FACTORY_MODE_TIME_INTERVAL (100)

TestLoop::TestLoop(QObject *parent) : QObject(parent)
{
    // 连接定时器超时信号到第二个任务

}

TestLoop::~TestLoop()
{
    qDebug()<<"==================== Test Loop begin ====================" ;

    qDebug()<<"==================== Test Loop end ====================" ;
}

void TestLoop::TestTaskInit()
{
    qDebug()<<"============ buildConfig Init ===============" ;
    // 上电初始控制阀 读取配置文件
    DeviceCLInfo = readXmlToStruct( "./buildConfig.xml" ) ;
}

void TestLoop::TestTaskDeinit()
{

}

void TestLoop::onControlAllValveTool()
{
    QByteArray qbyData ;
    bool ok = false ;
    for(int i=0;i<DeviceCLInfo.size();i++)
    {
        int value = DeviceCLInfo.at(i).address.toInt(&ok, 16); // value为161，即0xA1
        qbyData = GT_ModbusHandler.GT_ModbusWrite(value, 0x03, 0x4045, NULL, 0, NULL);
        emit sendMethodToSerial( qbyData );
        QThread::msleep(1000) ;
    }
}

void TestLoop::onReadAllGTDevicePressure(QList<DeviceInfo> list)
{
    if( list.isEmpty() )
    {
        return ;
    }
    for( int i=0;i<list.size();i++ )
    {
        QByteArray slaveID = list.at(i).slaveID ;
        qDebug()<<"加糖设备从机地址 - "<<slaveID ;
        emit sendMethodToSerial( onSetGTDeviceInFactoryMode( slaveID ) ) ;
        QThread::msleep(FACTORY_MODE_TIME_INTERVAL) ;
    }

    for( int i=0;i<DeviceCLInfo.size();i++ )
    {
        QByteArray data = onSetCTLDeviceCloseVal( DeviceCLInfo.at(i).address.toUtf8() );
        HexPrintf(data) ;
        emit sendMethodToSerial(data) ;
        QThread::msleep(100) ;
    }
}

QByteArray TestLoop::onSetGTDeviceInFactoryMode(QByteArray address )
{
    bool ok = false ;
    QByteArray qbyData ;
    int value = address.toInt(&ok, 16); // GT 设备地址提取
    uint8_t factoryModeBuffer[7] = "rcyigb";
    factoryModeBuffer[6] = 0x01 ;
    qbyData = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, 0x4047, factoryModeBuffer, sizeof(factoryModeBuffer), NULL);
    return qbyData ;
}

QByteArray TestLoop::onSetCTLDeviceCloseVal(QByteArray address)
{
    bool ok = false ;
    QByteArray ret ;
    int value = address.toInt(&ok, 16); // value为161，即0xA1
    uint8_t buffer[1] = { 0x01 };
    ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, 0x2001, buffer , 1 , NULL);
    return ret ;
}




