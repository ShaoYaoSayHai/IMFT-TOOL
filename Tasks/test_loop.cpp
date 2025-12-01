#include "test_loop.h"
#include <QDebug>
#include <QThread>
#include "hexprintf.h"

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
        QByteArray slaveID = list.at(i).SN ;
        qDebug()<<"加糖设备从机地址 - "<<slaveID ;
    }
}

