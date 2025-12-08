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
    if (pxTimerReadTimeout) {
        if (pxTimerReadTimeout->isActive()) {
            pxTimerReadTimeout->stop();
        }
        pxTimerReadTimeout->deleteLater();
    }
    qDebug() << "==================== Test Loop end ====================";
}

void TestLoop::TestTaskInit() {
    qDebug() << "==================== buildConfig Init ====================";
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
    if (!readInputControlSwitch("./buildConfig.xml", a1Addr, a2Addr)) {
        qDebug() << "读取输入控制开关失败";
        return;
    }
    if (!readPressureTimeoutConfig("./buildConfig.xml", readTimeout1,
                                   readTimeout2)) {
        qDebug() << "读取压力超时时间失败";
        return;
    }

    pxTimerReadTimeout = new QTimer();

    connect(pxTimerReadTimeout, &QTimer::timeout, this,
            &TestLoop::TimerReadTimeoutCallback);

    // 初始化队列
    commandList = parseXmlFile("./buildConfig.xml");

    // 遍历结果
    for (const CommandParams &cmd : commandList) {
        qDebug() << "ID:" << cmd.id << " Type: " << cmd.commandType
                 << " Slave IDs:" << cmd.slave_id << " - enable : " << cmd.enable
                 << " - writeBuffer:" << cmd.write_buffer.toHex()
                 << " - func:" << cmd.func << " - address"
                 << QString::number(cmd.address, 16);
    }
}

void TestLoop::TestTaskDeinit() {}

void TestLoop::TimerReadTimeoutCallback() {
    // 读取压力数据超时任务
    emit readPressureComplete();
}

void TestLoop::ControlTaskListInit() {
    // 初始化队列
    commandList = parseXmlFile("./buildConfig.xml");
    // 遍历结果
    for (const CommandParams &cmd : commandList) {
        qDebug() << "ID:" << cmd.id << " Type: " << cmd.commandType
                 << " Slave IDs:" << cmd.slave_id << " - enable : " << cmd.enable
                 << " - writeBuffer:" << cmd.write_buffer.toHex()
                 << " - func:" << cmd.func << " - address"
                 << QString::number(cmd.address, 16);
    }
}

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

#if 0
#else
    //
    bool ok = false;
    QByteArray qbyData;
    int value = address.toInt(&ok, 10); // GT 设备地址提取
    uint8_t factoryModeBuffer[1];
    factoryModeBuffer[0] = 0x01;
    qbyData = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, REG_GT_FACTORY,
                                              factoryModeBuffer,
                                              sizeof(factoryModeBuffer), NULL);
    return qbyData;

#endif
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
    uint8_t factoryModeBuffer[1];
    factoryModeBuffer[0] = 0x00;
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
    uint16_t regReadSwitchAddr = 0x4033;
    // 遍历XML文件内容 获取到清除异常地址
    for (int i = 0; i < commandList.size(); i++) {
        if (commandList.at(i).commandType.contains("CommandReadValveStatus")) {
            regReadSwitchAddr = commandList.at(i).address;
            break;
        }
    }
    ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x03, regReadSwitchAddr, NULL, 0,
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
    uint16_t regClearAddress = 0x5080;
    // 遍历XML文件内容 获取到清除异常地址
    for (int i = 0; i < commandList.size(); i++) {
        if (commandList.at(i).commandType.contains("CommandClearAllError")) {
            regClearAddress = commandList.at(i).address;
            break;
        }
    }
    uint8_t resetErrorValue = 0x01 ;
    ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, regClearAddress, &resetErrorValue, 1,
                                          NULL);
    return ret;
}

void TestLoop::GT_ResetDeviceErrorAll(QList<DeviceInfo> list) {
    // 先读取大气压
    for (int i = 0; i < list.size(); i++) {
        emit sendMethodToSerial(
                    GT_BuildDeviceErrorClear(list.at(i).slaveID)); // 发送大气压读取
        QThread::msleep(50);
    }
}

void TestLoop::GT_EnterFactoryModeAll(QList<DeviceInfo> list) {

#if 0
    for (int i = 0; i < list.size(); i++) {
        emit sendMethodToSerial(
                    GT_BuildDeviceFactoryModeEnter(list.at(i).slaveID)); // 发送大气压读取
        QThread::msleep(200);

        emit logCurrentStep((list.at(i).SN + "进入产测模式"));
    }
#else
    for (int i = 0; i < list.size(); i++) {
        emit sendMethodToSerial(
                    GT_BuildDeviceFactoryModeEnter(list.at(i).slaveID)); // 进入产测模式
        QThread::msleep(200);

        emit logCurrentStep((list.at(i).SN + "进入产测模式"));
    }
#endif

}

void TestLoop::GT_ExitFactoryModeAll(QList<DeviceInfo> list) {
    for (int i = 0; i < list.size(); i++) {
        emit sendMethodToSerial(
                    GT_BuildDeviceFactoryModeExit(list.at(i).slaveID)); // 发送大气压读取
        QThread::msleep(200);

        emit logCurrentStep((list.at(i).SN + "退出产测模式"));
    }
}

QByteArray TestLoop::GT_SetSwitchModeInClosed(QByteArray address)
{
    bool ok = false;
    QByteArray ret;
    int value = address.toInt(&ok, 10); // slaveID
    uint16_t reg = 0x5084;
    // 遍历XML文件内容 获取到清除异常地址
    for (int i = 0; i < commandList.size(); i++) {
        if (commandList.at(i).commandType.contains("CommandSetSwitchMode")) {
            reg = commandList.at(i).address;
            break;
        }
    }
    uint8_t setValue = 0x01 ;
    ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, reg, &setValue, 1,
                                          NULL);
    return ret ;
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
        QThread::msleep(readAirPressureTime);
    }
    // 然后读取阀前压力
    for (int i = 0; i < list.size(); i++) {
        emit sendMethodToSerial(
                    GT_BuildDeviceFaqianPressure(list.at(i).slaveID)); // 阀前读取
        QThread::msleep(readFaqianPressureTime);
    }
    // 在此处执行一个定时器更合适
    pxTimerReadTimeout->setSingleShot(true);
    if (!pxTimerReadTimeout->isActive()) {
        pxTimerReadTimeout->start(readTimeout1); // 设置5s超时
    } else {
        pxTimerReadTimeout->setSingleShot(true);
        pxTimerReadTimeout->stop();
        pxTimerReadTimeout->start(readTimeout1); // 设置5s超时
    }
}

QByteArray TestLoop::CTL_BuildDeviceSwitchClose(QByteArray address) {
    bool ok = false;
    QByteArray ret;
    int value = address.toInt(&ok, 16); //
    uint8_t buffer[1] = {0x00};
    ret = GT_ModbusHandler.GT_ModbusWrite(value, 0x06, 0x2001, buffer, 1, NULL);
    return ret;
}

QByteArray TestLoop::CTL_BuildDeviceSwitchOpen(QByteArray address) {
    bool ok = false;
    QByteArray ret;
    int value = address.toInt(&ok, 16); //
//    qDebug() << "DEBUG +++ slaveID" << value;
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

        emit logCurrentStep("控制阀" + data[i].address.toUtf8() + "已经动作完毕");
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

void TestLoop::CTL_DeviceSwitchOpenAll() {
    // DeviceCLInfo
    qDebug() << "地址" << DeviceCLInfo[0].address.toUtf8();
    for (int i = 0; i < DeviceCLInfo.size(); i++) {
        emit sendMethodToSerial(
                    CTL_BuildDeviceSwitchOpen(DeviceCLInfo[i].address.toUtf8()));
        QThread::msleep(this->switchResetTime);

        emit logCurrentStep(
                    (DeviceCLInfo.at(i).address.toUtf8() + "开启指令已经发送"));
    }
}

void TestLoop::CTL_DeviceSwitchCloseAll() {
    // DeviceCLInfo
    for (int i = 0; i < DeviceCLInfo.size(); i++) {
        emit sendMethodToSerial(
                    CTL_BuildDeviceSwitchClose(DeviceCLInfo[i].address.toUtf8()));
        QThread::msleep(this->switchResetTime);

        emit logCurrentStep(
                    (DeviceCLInfo.at(i).address.toUtf8() + "关闭指令已经发送"));
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
void TestLoop::CTL_SetInputControlDeviceSwitchOpen(QByteArray address) {
    emit sendMethodToSerial(
                CTL_BuildDeviceSwitchOpen(address)); // 输入参数必须是QByteArray
    QThread::msleep(this->switchResetTime);
}

/**
 * @brief 控制输入控制设备开关操作 关闭所有的阀门
 * @param address
 */
void TestLoop::CTL_SetInputControlDeviceSwitchClose(QByteArray address) {
    QByteArray data;
    data.append(address);
    emit sendMethodToSerial(
                CTL_BuildDeviceSwitchClose(data)); // 输入参数必须是QByteArray
    QThread::msleep(this->switchResetTime);
}

/**
 * @brief
 *        打开进气端阀门
 */
void TestLoop::LST_CommandOpenInputSwitch2KPa() {
    for (int i = 0; i < commandList.size(); i++) {
        if (commandList.at(i).commandType == "commandOpenInputValve") {
            for (int j = 0; j < commandList.at(i).slave_id.size(); j++) {
                emit logCurrentStep(
                            "打开控制阀 " +
                            QByteArray::number((commandList.at(i).slave_id.at(j)), 16));
                CTL_SetInputControlDeviceSwitchOpen(
                            QByteArray::number((commandList.at(i).slave_id.at(j)), 16));
            }
        }
    }
}

void TestLoop::LST_CommandCloseInputSwitch2KPa()
{
    for (int i = 0; i < commandList.size(); i++) {
        if (commandList.at(i).commandType == "commandCloseInputValve") {
            for (int j = 0; j < commandList.at(i).slave_id.size(); j++) {
                emit logCurrentStep(
                            "关闭控制阀 " +
                            QByteArray::number((commandList.at(i).slave_id.at(j)), 16));
                CTL_SetInputControlDeviceSwitchOpen(
                            QByteArray::number((commandList.at(i).slave_id.at(j)), 16));
            }
        }
    }
}


void TestLoop::LST_CommandPollingOpenAllSwitch()
{
    for (int i = 0; i < commandList.size(); i++) {
        if (commandList.at(i).commandType == "CommandOpenFireValveNED") {
            for (int j = 0; j < commandList.at(i).slave_id.size(); j++) {
                emit logCurrentStep(
                            "模拟点火开阀 " +
                            QByteArray::number((commandList.at(i).slave_id.at(j)), 16));
                CTL_SetInputControlDeviceSwitchOpen(
                            QByteArray::number((commandList.at(i).slave_id.at(j)), 16));
                QThread::msleep(this->switchOpenTime);
                CTL_SetInputControlDeviceSwitchClose(
                            QByteArray::number((commandList.at(i).slave_id.at(j)), 16));
                QThread::msleep(this->switchCloseTime);
            }
        }
    }
}

/**
 * @brief 读取所有阀门状态
 * @param data
 */
void TestLoop::LST_CommandPollReadSwitch(QList<DeviceInfo> data)
{
    // 先读取大气压
    for (int i = 0; i < data.size(); i++) {
        emit sendMethodToSerial(
                    GT_BuildDeviceSwitch(data.at(i).slaveID)); // 发送大气压读取
        QThread::msleep(50);
    }
}

void TestLoop::LST_CommandPollSetSwitchMode(QList<DeviceInfo> data)
{
    // 先读取大气压
    for (int i = 0; i < data.size(); i++) {
        emit sendMethodToSerial(
                    GT_SetSwitchModeInClosed(data.at(i).slaveID)); // 发送常开模式设置
        QThread::msleep(50);
    }
}

void TestLoop::DO_TaskCheckLowPressure(QList<DeviceInfo> data) {
    // 进入产测模式
    emit logCurrentStep(
                "========================= 欠压功能测试 BEGIN =========================");
    emit logCurrentStep("进入产测模式");
    GT_EnterFactoryModeAll(data);
    emit logCurrentStep("即将开启所有控制阀");
    // 执行所有阀门打开
    CTL_DeviceSwitchOpenAll();
    emit logCurrentStep("读取所有阀前压力");
    // 执行所有压力读取
    GT_ReadListAllPressure(data);
    // 关闭所有阀门
    emit logCurrentStep("关闭所有阀门");
    CTL_DeviceSwitchCloseAll();
    // 清除所有异常信息
    //  GT_ResetDeviceErrorAll(data);
    // 退出产测模式
    emit logCurrentStep("退出产测模式");
    GT_ExitFactoryModeAll(data);
    emit logCurrentStep(
                "========================= 欠压功能测试 END =========================");
}

void TestLoop::DO_TaskOpenFire(QList<DeviceInfo> data) {

    emit logCurrentStep(
                "========================= 点火开阀测试 BEGIN =========================");
    // 执行2KPa阀门开启 发送打开阀门指令
    LST_CommandOpenInputSwitch2KPa();
    // 此时的管道供气2KPa，然后等待几秒钟时间，在这几秒钟准备执行清除异常
    for (int i = 0; i < GAS_SMOOTH_TIME; i++) {
        emit logCurrentStep("等待压力稳定 " + QByteArray::number(GAS_SMOOTH_TIME - i));
        QThread::msleep(1000); // 等待1s
    }
    // 清除所有异常信息
    GT_ResetDeviceErrorAll(data);
    for (int i = 0; i < GAS_SMOOTH_TIME; i++) {
        emit logCurrentStep("等待压力稳定 " + QByteArray::number(GAS_SMOOTH_TIME - i));
        QThread::msleep(1000); // 等待1s
    }
    // 模拟点火开阀
    LST_CommandPollingOpenAllSwitch();
    QThread::msleep(2000);
    // 读取所有阀门状态
    GT_ReadDeviceSwitchStatusAll( data );
    // 关闭进气端阀门
    LST_CommandCloseInputSwitch2KPa();

#if 0
    // 进气端B1,此时供气2KPa
    //    qDebug()<<"A1 地址 - "<<a1Addr ;
    QByteArray B1Addr = QByteArray::number( a1Addr , 16 );
    emit logCurrentStep("========================= 点火开阀测试 BEGIN =========================") ;
    emit logCurrentStep("打开控制阀 "+B1Addr);
    CTL_SetInputControlDeviceSwitchOpen( B1Addr );
    // 清除异常
    // 暂时不写
    emit logCurrentStep("等待压力稳定 3");
    QThread::msleep(1000); // 等待2s
    emit logCurrentStep("等待压力稳定 2");
    QThread::msleep(1000); // 等待2s
    emit logCurrentStep("等待压力稳定 1");
    QThread::msleep(1000); // 等待2s
    // // 执行所有压力读取
    // GT_ReadListAllPressure(data); // 读取所有压力
    // 依次开启阀门并关闭
    emit logCurrentStep("依次开启控制阀");
    CTL_SetDeviceSwitchOperate(DeviceCLInfo);
    QThread::msleep(1000); // 等待1s
    // 读取所有阀门状态
    emit logCurrentStep("读取阀门开启状态");
    GT_ReadDeviceSwitchStatusAll( data );
    QThread::msleep(1000); // 等待1s
    // 关闭进气端阀门
    emit logCurrentStep("关闭进气端阀门") ;
    CTL_SetInputControlDeviceSwitchClose(B1Addr) ;
#else
    // commandList

#endif
    emit logCurrentStep(
                "========================= 点火开阀测试 END =========================");
}

void TestLoop::DO_TaskOverPressure(QList<DeviceInfo> data) {
    emit logCurrentStep(
                "========================= 超压功能测试 BEGIN =========================");
    // 进入产测模式
    emit logCurrentStep("进入产测模式");
    GT_EnterFactoryModeAll(data);
    emit logCurrentStep("关闭所有阀门");
    CTL_DeviceSwitchCloseAll();

    QByteArray Addr = QByteArray::number(a2Addr, 16);
    emit logCurrentStep("开启高压进气端阀门" + Addr);
    CTL_SetInputControlDeviceSwitchOpen(Addr);

    for (int i = 0; i < GAS_SMOOTH_TIME; i++) {
        emit logCurrentStep("等待压力稳定 " + QByteArray::number(GAS_SMOOTH_TIME - i));
        QThread::msleep(1000); // 等待1s
    }

    // // 执行所有压力读取
    emit logCurrentStep("读取所有压力数据");
    GT_ReadListAllPressure(data); // 读取所有压力
    // 关闭进气端阀门
    emit logCurrentStep("关闭进气端阀门" + Addr);
    CTL_SetInputControlDeviceSwitchClose(Addr);
    // 退出产测模式
    emit logCurrentStep("退出产测模式");
    GT_ExitFactoryModeAll(data);
    emit logCurrentStep(
                "========================= 超压功能测试 END =========================");
    // 设置常开模式
//    LST_CommandPollSetSwitchMode( data );
}

void TestLoop::DO_SubmitInfoToMES(QList<DeviceInfo> data) {
    for (DeviceInfo &device : data) {
        QString sn = device.SN;
        QString info = InfoParser::generateXmlString(sn, "OMFT");
        emit logCurrentStep("GET -- " + info.toUtf8());
        emit sendHttpParam( info );
        QThread::msleep(200) ;
    }
}

/**
 * @brief 读取所有阀门状态
 * @param data
 */
void TestLoop::GT_ReadDeviceSwitchStatusAll(QList<DeviceInfo> data) {
    for (int i = 0; i < data.size(); i++) {
        emit sendMethodToSerial(GT_BuildDeviceSwitch(data[i].slaveID));
        QThread::msleep(this->switchResetTime);
        emit logCurrentStep("待检品" + data[i].SN + "读取状态");
    }
}
