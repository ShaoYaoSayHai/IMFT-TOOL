#include "test_loop.h"
#include "hexprintf.h"
#include <QDebug>
#include <QThread>

#define FACTORY_MODE_TIME_INTERVAL (100)

TestLoop::TestLoop(QObject *parent) : QObject(parent) {
  // 连接定时器超时信号到第二个任务
}

TestLoop::~TestLoop() {
  qDebug() << "==================== Test Loop begin ====================";

  qDebug() << "==================== Test Loop end ====================";
}

void TestLoop::TestTaskInit() {
  qDebug() << "============ buildConfig Init ===============";
  // 上电初始控制阀 读取配置文件
  DeviceCLInfo = readXmlToStruct("./buildConfig.xml");

  if (!readSwitchTimesWithDebug("./buildConfig.xml", switchOpenTime,
                                switchCloseTime, switchResetTime)) {
    qDebug() << "读取开关时间失败";
    return;
  }

  if (!readPressureTimeConfig("./buildConfig.xml", readAirPressureTime,
                              readFaqianPressureTime, readFahouPressureTime)) {
    qDebug() << "读取压力时间失败";
    return;
  }

  uint8_t a1Addr = 0, a2Addr = 0;
  if (!readInputControlSwitch("./buildConfig.xml", a1Addr, a2Addr)) {
    qDebug() << "读取输入控制开关失败";
    return;
  }
  qDebug() << "a1Addr:" << a1Addr << "a2Addr:" << a2Addr;
}

void TestLoop::TestTaskDeinit() {}

void TestLoop::onControlAllValveTool() {
  QByteArray qbyData;
  bool ok = false;
  for (int i = 0; i < DeviceCLInfo.size(); i++) {
    int value = DeviceCLInfo.at(i).address.toInt(&ok, 10);
    qbyData = GT_ModbusHandler.GT_ModbusWrite(value, 0x03, REG_GT_READSN, NULL,
                                              0, NULL);
    emit sendMethodToSerial(qbyData);
    QThread::msleep(1000);
  }
}

void TestLoop::onReadAllGTDevicePressure(QList<DeviceInfo> list) {
  if (list.isEmpty()) {
    return;
  }
  for (int i = 0; i < list.size(); i++) {
    QByteArray slaveID = list.at(i).slaveID;
    qDebug() << "加糖设备从机地址 - " << slaveID;
    emit sendMethodToSerial(GT_BuildDeviceFactoryModeEnter(slaveID));
    QThread::msleep(FACTORY_MODE_TIME_INTERVAL);
  }

  for (int i = 0; i < DeviceCLInfo.size(); i++) {
    QByteArray data =
        CTL_BuildDeviceSwitchClose(DeviceCLInfo.at(i).address.toUtf8());
    HexPrintf(data);
    emit sendMethodToSerial(data);
    QThread::msleep(100);
  }
}

/**
 * @brief 进入产测模式
 * @param address
 * @return
 */
QByteArray TestLoop::GT_BuildDeviceFactoryModeEnter(QByteArray address) {
  bool ok = false;
  QByteArray qbyData;
  int value = address.toInt(&ok, 10); // GT 设备地址提取
  uint8_t factoryModeBuffer[7] = "rcyigb";
  factoryModeBuffer[6] = 0x01;
  qbyData = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, REG_GT_FACTORY,
                                            factoryModeBuffer,
                                            sizeof(factoryModeBuffer), NULL);
  return qbyData;
}

/**
 * @brief 退出产测模式
 * @param address
 * @return
 */
QByteArray TestLoop::GT_BuildDeviceFactoryModeExit(QByteArray address) {
  bool ok = false;
  QByteArray qbyData;
  int value = address.toInt(&ok, 10); // GT 设备地址提取
  uint8_t factoryModeBuffer[7] = "rcyigb";
  factoryModeBuffer[6] = 0x00;
  qbyData = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, REG_GT_FACTORY,
                                            factoryModeBuffer,
                                            sizeof(factoryModeBuffer), NULL);
  return qbyData;
}

/**
 * @brief 读取加糖板子的限位开关状态
 * @param address
 * @return
 */
QByteArray TestLoop::GT_BuildDeviceSwitch(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 10); // slaveID
  ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x03, REG_GT_SWITCH, NULL, 0,
                                        NULL);
  return ret;
}

/**
 * @brief 读取加糖电路板的大气压值
 * @param address
 * @return
 */
QByteArray TestLoop::GT_BuildDeviceAirPressure(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 10); // slaveID
  ret =
      GT_ModbusHandler.GT_ModbusWrite(value, 0x03, REG_GT_AIRP, NULL, 0, NULL);
  return ret;
}

/**
 * @brief 读取阀门前的压力传感器的值
 * @param address
 * @return
 */
QByteArray TestLoop::GT_BuildDeviceFaqianPressure(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 10); // slaveID
  ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x03, REG_GT_FAQIAN, NULL, 0,
                                        NULL);
  return ret;
}

/**
 * @brief 读取阀门后的压力传感器的值
 * @param address
 * @return
 */
QByteArray TestLoop::GT_BuildDeviceFahouPressure(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 10); // slaveID
  ret =
      GT_ModbusHandler.GT_ModbusWrite(value, 0x03, REG_GT_FAHOU, NULL, 0, NULL);
  return ret;
}

/**
 * @brief 清除板端的异常信息
 * @param address
 * @return
 */
QByteArray TestLoop::GT_BuildDeviceErrorClear(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 10); // slaveID
  ret =
      GT_ModbusHandler.GT_ModbusWrite(value, 0x03, REG_GT_CLEAR, NULL, 0, NULL);
  return ret;
}

void TestLoop::onTestBaseCmdAll() {
  QThread::msleep(1000);
  emit sendMethodToSerial(GT_BuildDeviceFactoryModeEnter("01"));
  QThread::msleep(1000);
  emit sendMethodToSerial(GT_BuildDeviceErrorClear("01"));
  QThread::msleep(1000);
  emit sendMethodToSerial(GT_BuildDeviceAirPressure("01"));
  QThread::msleep(1000);
  emit sendMethodToSerial(GT_BuildDeviceFahouPressure("01"));
  QThread::msleep(1000);
  emit sendMethodToSerial(GT_BuildDeviceFaqianPressure("01"));
  QThread::msleep(1000);
  emit sendMethodToSerial(GT_BuildDeviceSwitch("01"));
  QThread::msleep(1000);
  emit sendMethodToSerial(GT_BuildDeviceFactoryModeExit("01"));
  QThread::msleep(1000);
  emit sendMethodToSerial(CTL_BuildDeviceSwitchOpen("01"));
  QThread::msleep(1000);
  emit sendMethodToSerial(CTL_BuildDeviceSwitchClose("01"));
}

/**
 * @brief 读取所有压力值
 * @param list
 */
void TestLoop::GT_ReadListAllPressure(QList<DeviceInfo> list) {
  // 先读取大气压
  for (int i = 0; i < list.size(); i++) {
    emit sendMethodToSerial(
        GT_BuildDeviceAirPressure(list.at(i).slaveID)); // 发送大气压读取
    QThread::msleep(100);
    //    emit sendMethodToSerial(
    //        GT_BuildDeviceFaqianPressure(list.at(i).slaveID)); // 阀前读取
    //    QThread::msleep(100);
    // emit sendMethodToSerial(
    //     GT_BuildDeviceFahouPressure(list.at(i).slaveID)); // 阀后读取
    // QThread::msleep(100);
  }
  // 然后读取阀前压力
  for (int i = 0; i < list.size(); i++) {
    emit sendMethodToSerial(
        GT_BuildDeviceFaqianPressure(list.at(i).slaveID)); // 阀前读取
    QThread::msleep(100);
  }

  QThread::msleep(1000);
  emit readPressureComplete(); // 读取压力执行完毕
}

QByteArray TestLoop::CTL_BuildDeviceSwitchClose(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 10); //
  uint8_t buffer[1] = {0x00};
  ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, 0x2001, buffer, 1, NULL);
  return ret;
}

QByteArray TestLoop::CTL_BuildDeviceSwitchOpen(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 10); //
  uint8_t buffer[1] = {0x01};
  ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, 0x2001, buffer, 1, NULL);
  return ret;
}

/**
 * @brief 控制设备开关操作 按照顺序去开关
 * @param data
 */
void TestLoop::CTL_SetDeviceSwitchOperate(QList<CLTDeviceInfo> data) {
  // 这两个时间是开关阀的时间
  for (int i = 0; i < data.size(); i++) {
    emit sendMethodToSerial(
        CTL_BuildDeviceSwitchOpen(data[i].address.toUtf8()));
    QThread::msleep(this->switchOpenTime);
    emit sendMethodToSerial(
        CTL_BuildDeviceSwitchClose(data[i].address.toUtf8()));
    QThread::msleep(this->switchCloseTime);
  }
  // 模拟点火开阀执行完成
}

/**
 * @brief 控制设备开关操作 按照顺序去关闭所有阀门
 * @param data
 */
void TestLoop::CTL_SetDeviceSwitchCloseAll(QList<CLTDeviceInfo> data) {
  for (int i = 0; i < data.size(); i++) {
    emit sendMethodToSerial(
        CTL_BuildDeviceSwitchClose(data[i].address.toUtf8()));
    QThread::msleep(this->switchResetTime);
  }
}

/**
 * @brief 控制设备开关操作 打开所有的阀门
 * @param data
 */
void TestLoop::CTL_SetDeviceSwitchOpenAll(QList<CLTDeviceInfo> data) {
  for (int i = 0; i < data.size(); i++) {
    emit sendMethodToSerial(
        CTL_BuildDeviceSwitchOpen(data[i].address.toUtf8()));
    QThread::msleep(this->switchResetTime);
  }
}

/**
 * @brief 控制输入控制设备开关操作 打开所有的阀门
 * @param address
 */
void TestLoop::CTL_SetInputControlDeviceSwitchOpen(uint8_t address) {
  QByteArray data;
  data.append(address);
  emit sendMethodToSerial(
      CTL_BuildDeviceSwitchOpen(data)); // 输入参数必须是QByteArray
  QThread::msleep(this->switchResetTime);
}

/**
 * @brief 控制输入控制设备开关操作 打开所有的阀门
 * @param address
 */
void TestLoop::CTL_SetInputControlDeviceSwitchClose(uint8_t address) {
  QByteArray data;
  data.append(address);
  emit sendMethodToSerial(
      CTL_BuildDeviceSwitchClose(data)); // 输入参数必须是QByteArray
  QThread::msleep(this->switchResetTime);
}

/**
 * @brief 读取所有阀门状态
 * @param data
 */
void TestLoop::GT_ReadDeviceSwitchStatusAll(QList<DeviceInfo> data) {
  for (int i = 0; i < data.size(); i++) {
    emit sendMethodToSerial(GT_BuildDeviceSwitch(data[i].slaveID));
    QThread::msleep(this->switchResetTime);
  }
}
