#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QLabel>

#include "./SerialSource/serialworker.h"
#include "./SerialSource/base_serial_port.h"

#include "./hexprintf.h"

#include "./Table/tablecontrol.h"

#include "./Modbus/ModbusReMapping.h"
#include "./Modbus/gt_modbus.h"
#include "./Logs/logs.h"

#include <QTimer>

#include "./FileReadWrite/filerw.h"

#include "./Tasks/testworker.h"



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    SerialWorker *pxSerialWorkerUART_Handler = new SerialWorker(this);

    TableControl *pxTable = nullptr ;

    Logs *pxBrowserLogs = nullptr ;

    GT_Modbus GT_ModbusHandler ;

    // 设备列表
    QList<DeviceInfo> GT_DeviceList ;
    // 定时器 用作发送0xAA 和 0x4045读取对应的SN号
    QTimer *pxTimerReadSN = new QTimer(this) ;
    // 任务线程测试任务
    TestWorker *pxTestWorkerHandler = new TestWorker(this) ;

public slots:

    //串口初始化
    void SerialWorkerInit();
    // 串口读取数据回调函数
    void onSerialWorkerReadData(const QByteArray &data);
    // 连接状态更新
    void onConnectStatusChanged(int status);

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // ++++++++++++++++++++++++++ 交互类函数 ++++++++++++++++++++++++++++++++++
    // 回收函数
    void ReleaseUiClasses();
    // 串口列表刷新
    void RefreshSerialPorts();
    // GUI Table 初始化函数
    void GUI_TableInit();
    // 读取SN号的定时器函数
    void onTimerTimeoutReadSN();
    // 主动向所有的从机控制阀发送信号
    void sendDeviceControlSignal( int slaveID , int status );

private slots:

    void on_pushButton_PortOpen_clicked();
    void on_pushButton_PortRefresh_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_lineEdit_textChanged(const QString &arg1);
};
#endif // MAINWINDOW_H
