#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QTimer>

#include "./FileReadWrite/filerw.h"
#include "./Logs/logs.h"
#include "./Modbus/ModbusReMapping.h"
#include "./Modbus/gt_modbus.h"
#include "./SerialSource/base_serial_port.h"
#include "./SerialSource/serialworker.h"
#include "./Table/tablecontrol.h"
#include "./Tasks/testworker.h"
#include "./hexprintf.h"
#include "./HttpClient/httpclient.h"
#include "./HttpClient/infoparse.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;

  SerialWorker *pxSerialWorkerUART_Handler = new SerialWorker(this);

  TableControl *pxTable = nullptr;
  // 日志
  Logs *pxBrowserLogs = nullptr;
  // 设备列表
  QList<DeviceInfo> GT_DeviceList;
  // 定时器 用作发送0xAA 和 0x4045读取对应的SN号
  QTimer *pxTimerReadSN = new QTimer(this);
  // 任务线程测试任务
  TestWorker *pxTestWorkerHandler = new TestWorker(this);
  // 超压欠压执行标志位
  DoTestType DoTestFlag = {false, false};
  // HTTP客户端操作
  HttpClient *pxHttpClient = new HttpClient( this );

public slots:

  // 串口初始化
  void SerialWorkerInit();
  // 串口读取数据回调函数
  void onSerialWorkerReadData(const QByteArray &data);
  // 连接状态更新
  void onConnectStatusChanged(int status);
  // 恢复按钮操作
  void ResetButtonEnable();
  void SetButtonDisable();

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
  void sendDeviceControlSignal(int slaveID, int status);
  // 更新Table内容
  void onTableMapping();
  // 复位超压欠压点火开阀测试项目
  void onResetTestFlag();

private slots:

  void on_pushButton_PortOpen_clicked();
  void on_pushButton_PortRefresh_clicked();
  void on_pushButton_5_clicked();
  void on_pushButton_6_clicked();
  void on_lineEdit_textChanged(const QString &arg1);
  void on_pushButton_clicked();
  void on_pushButton_3_clicked();
  void on_pushButton_2_clicked();
  void on_pushButton_4_clicked();
};
#endif // MAINWINDOW_H
