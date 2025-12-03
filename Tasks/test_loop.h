// SPDX-License-Identifier: LGPL-3.0-only
// 许可声明：本文件依据 GNU LGPL v3 许可条款分发与使用。
// 完整许可文本见项目根目录：LPGL3.md

#ifndef TESTTASK_H
#define TESTTASK_H

#include <QWidget>

#include "./FileReadWrite/filerw.h"
#include "./Modbus/gt_modbus.h"
#include <QTimer>

class TestLoop : public QObject {
  Q_OBJECT
public:
  explicit TestLoop(QObject *parent = nullptr);

  ~TestLoop();

  // 根据XML文件读取到的设备信息
  QList<CLTDeviceInfo> DeviceCLInfo;
  // 阀门开启的保持时间
  int switchOpenTime = 1500;
  int switchCloseTime = 1000;
  int switchResetTime = 200;

  // 读取压力的时间间隔
  int readAirPressureTime = 100;
  int readFaqianPressureTime = 100;
  int readFahouPressureTime = 100;

  GT_Modbus GT_ModbusHandler;

private:
public slots:

  void TestTaskInit();

  void TestTaskDeinit();

  // 控制所有电磁阀
  void onControlAllValveTool();
  // 批量执行
  void onReadAllGTDevicePressure(QList<DeviceInfo> list);
  // 执行单次的产测模式进入
  QByteArray GT_BuildDeviceFactoryModeEnter(QByteArray address);
  // 产测模式退出
  QByteArray GT_BuildDeviceFactoryModeExit(QByteArray address);
  // 读取加糖PCB的限位开关 0x4033
  QByteArray GT_BuildDeviceSwitch(QByteArray address);
  // 读取加糖PCB的大气压值 4001
  QByteArray GT_BuildDeviceAirPressure(QByteArray address);
  // 读取加糖PCB的前端压力 4010
  QByteArray GT_BuildDeviceFaqianPressure(QByteArray address);
  // 读取加糖PCB的后端压力 4012
  QByteArray GT_BuildDeviceFahouPressure(QByteArray address);
  // 清除加糖PCB的设备异常信息
  QByteArray GT_BuildDeviceErrorClear(QByteArray address);

  // 测试所有基础指令
  void onTestBaseCmdAll();
  // 执行所有的压力读取
  void GT_ReadListAllPressure(QList<DeviceInfo> list);
  // 读取所有阀门状态
  void GT_ReadDeviceSwitchStatusAll(QList<DeviceInfo> data);

  // ================================================================
  // 执行所有电磁阀关闭
  QByteArray CTL_BuildDeviceSwitchClose(QByteArray address);
  // 打开控制阀-电磁阀
  QByteArray CTL_BuildDeviceSwitchOpen(QByteArray address);

  // ====================== 所有阀门开启关闭指令 ===========================
  void CTL_SetDeviceSwitchOperate(QList<CLTDeviceInfo> data);
  // 依次关闭所有阀门
  void CTL_SetDeviceSwitchCloseAll(QList<CLTDeviceInfo> data);
  // 控制设备开关操作 打开所有的阀门
  void CTL_SetDeviceSwitchOpenAll(QList<CLTDeviceInfo> data);
  // 开启进气端阀门
  void CTL_SetInputControlDeviceSwitchOpen(uint8_t address);
  // 关闭进气端阀门
  void CTL_SetInputControlDeviceSwitchClose(uint8_t address);
signals:

  // 调用串口总线发送数据
  void sendMethodToSerial(const QByteArray &data);
  // 模拟点火开阀完成 执行下一步动作
  void simulateIgnitionComplete();
  // 读取压力完成
  void readPressureComplete();
};

#endif // TESTTASK_H
