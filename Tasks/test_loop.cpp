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
  qDebug() << "SwitchOpenTime:" << switchOpenTime;
  qDebug() << "SwitchCloseTime:" << switchCloseTime;
  qDebug() << "SwitchResetTime:" << switchResetTime;

  if (!readPressureTimeConfig("./buildConfig.xml", readAirPressureTime,
                              readFaqianPressureTime, readFahouPressureTime)) {
    qDebug() << "读取压力时间失败";
    return;
  }
  qDebug() << "readAirPressureTime:" << readAirPressureTime;
  qDebug() << "readFaqianPressureTime:" << readFaqianPressureTime;
  qDebug() << "readFahouPressureTime:" << readFahouPressureTime;
}

void TestLoop::TestTaskDeinit() {}

void TestLoop::onControlAllValveTool() {
  QByteArray qbyData;
  bool ok = false;
  for (int i = 0; i < DeviceCLInfo.size(); i++) {
    int value = DeviceCLInfo.at(i).address.toInt(&ok, 16); // value为161，即0xA1
    qbyData =
        GT_ModbusHandler.GT_ModbusWrite(value, 0x03, 0x4045, NULL, 0, NULL);
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
  int value = address.toInt(&ok, 16); // GT 设备地址提取
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
  int value = address.toInt(&ok, 16); // GT 设备地址提取
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
  int value = address.toInt(&ok, 16); // slaveID
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
  int value = address.toInt(&ok, 16); // slaveID
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
  int value = address.toInt(&ok, 16); // slaveID
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
  int value = address.toInt(&ok, 16); // slaveID
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
  int value = address.toInt(&ok, 16); // slaveID
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
//    emit sendMethodToSerial(GT_BuildDeviceAirPressure(list.at(i).address));
//    QThread::msleep(100);
//    emit sendMethodToSerial(GT_BuildDeviceFahouPressure(list.at(i).address));
//    QThread::msleep(100);
//    emit sendMethodToSerial(GT_BuildDeviceFaqianPressure(list.at(i).address));
//    QThread::msleep(100);
  }
}

QByteArray TestLoop::CTL_BuildDeviceSwitchClose(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 16); // value为161，即0xA1
  uint8_t buffer[1] = {0x00};
  ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, 0x2001, buffer, 1, NULL);
  return ret;
}

QByteArray TestLoop::CTL_BuildDeviceSwitchOpen(QByteArray address) {
  bool ok = false;
  QByteArray ret;
  int value = address.toInt(&ok, 16); // value为161，即0xA1
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
