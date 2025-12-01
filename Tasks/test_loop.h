// SPDX-License-Identifier: LGPL-3.0-only
// 许可声明：本文件依据 GNU LGPL v3 许可条款分发与使用。
// 完整许可文本见项目根目录：LPGL3.md

#ifndef TESTTASK_H
#define TESTTASK_H

#include <QWidget>

#include <QTimer>
#include "./Modbus/gt_modbus.h"
#include "./FileReadWrite/filerw.h"

class TestLoop : public QObject {
    Q_OBJECT
public:
    explicit TestLoop(QObject *parent = nullptr);

    ~TestLoop();

    // 根据XML文件读取到的设备信息
    QList<CLTDeviceInfo> DeviceCLInfo ;

private:

    GT_Modbus GT_ModbusHandler ;

public slots:

    void TestTaskInit();

    void TestTaskDeinit();

    // 控制所有电磁阀
    void onControlAllValveTool();
    // 批量执行
    void onReadAllGTDevicePressure( QList<DeviceInfo> list );
    // 执行单次的产测模式进入
    QByteArray onSetGTDeviceInFactoryMode(QByteArray address ) ;
    // 执行所有电磁阀关闭
    QByteArray onSetCTLDeviceCloseVal( QByteArray address );

signals:

    void sendMethodToSerial( const QByteArray &data );

};

#endif // TESTTASK_H
