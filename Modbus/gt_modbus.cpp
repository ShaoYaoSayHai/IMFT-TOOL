#include "gt_modbus.h"
#include "hexprintf.h"
#include <QDebug>

GT_Modbus::GT_Modbus(QObject *parent) : QObject(parent) {}

GT_Modbus::~GT_Modbus() {}

void GT_Modbus::GT_SetSlaveAddr(uint8_t slaveID) { this->slaveAddr = slaveID; }

uint8_t GT_Modbus::GT_GetSlaveAddr() { return this->slaveAddr; }

QByteArray GT_Modbus::GT_ModbusWrite(uint8_t slaveAddr, uint8_t funcCode,
                                     uint16_t regAddr, uint8_t *regValue,
                                     uint8_t regSize,
                                     ModbusReMappingFunc func) {
  QByteArray writeData;
  if (funcCode == 0x03) {
    writeData = BuildReadGtDevice(slaveAddr, regAddr);
  } else if (funcCode == 0x06) {
    writeData = BuildWriteGtDevice(slaveAddr, regAddr, regValue, regSize);
  }
  // 从机地址更新
  this->slaveAddr = slaveAddr;
  this->funcCode = funcCode;
  this->regAddr = regAddr;
//  qDebug() << "更新regAddr" << regAddr;
  // 解析函数修改
  xModbsuFunc = func;
  return writeData; // 返回需要传输的数据
}

int GT_Modbus::GT_RetMsgVerify(QByteArray data) {
  HexPrintf(data);
  GT_ReadInfo( data );
  // 是否是发出去的地址
  if ((uint8_t)(data.at(0)) != (this->slaveAddr)) {
    qDebug() << "从机地址错误" << (uint8_t)data.at(0);
  }
  if ((uint8_t)(data.at(1)) != funcCode) {
    qDebug() << "funcCode错误";
  }
  uint16_t regAddress =
      ((uint8_t)(data.at(2)) << 8) | ((uint8_t)(data.at(3)) & 0xFF);
  if (regAddress != regAddr) {
    qDebug() << "regAddress错误" << regAddr;
    //    return GT_Modbus::ERROR;
  }
  // 以上三点确认的是解析的是发出去的指令内容
  // CRC 校验准备
  QByteArray RET_Array = data.mid(data.size() - 2, data.size());
  uint8_t crc_l = (uint8_t)(RET_Array.at(1));
  uint8_t crc_h = (uint8_t)(RET_Array.at(0));
//  qDebug() << "L-" << crc_l << "H-" << crc_h;
  uint16_t RET_CRC = (crc_l << 8) | (crc_h & 0xFF);
  QByteArray result = data.mid(0, data.size() - 2);
  uint16_t CRC_Code = crc16_MODBUS(result);
//  qDebug() << "RET_CRC - " << RET_CRC << "-CRC Code - " << CRC_Code;
  if (RET_CRC != CRC_Code) {
//    return GT_Modbus::ERROR;
  }
  qbyData.clear();
  // 开始解析实际数据
  // 解析读取的返回值
  if (data.at(1) == 0x03) {
    qbyData = ParseGtDeviceReturnMsg(data);
    // HexPrintf(qbyData);
    /* 读取设备地址 */
    if (regAddress == 0xA0A0) {
      GT_SetSlaveAddr((uint8_t)(qbyData.at(0))); // 更新了从机地址
      return GT_Modbus::SUCCESS;
    } else if (regAddress == 0x4001) { /* 读取大气压 */
      int value = ParseGtAirPressureRead(qbyData);
      emit sig_updateAirPressure((uint8_t)(data.at(0)), AirPressureLocation::AIR, value);
    } else if (regAddress == 0x4010) { /* 读取进气端压力 */
      int value = ParseGtAirPressureRead(qbyData);
      emit sig_updateAirPressure((uint8_t)(data.at(0)), AirPressureLocation::INF, value);
    } else if (regAddress == 0x4012) { /* 读取出气端压力 */
      int value = ParseGtAirPressureRead(qbyData);
      emit sig_updateAirPressure((uint8_t)(data.at(0)), AirPressureLocation::END, value);
    } else if (regAddress == 0x4045) {     /* 读取SN号 */
      emit sig_updateSN(qbyData);          // 更新SN号
    } else if (regAddress == 0x4033) {     /* 读取阀门状态 */
      uint8_t valveStatus = qbyData.at(0); // 阀门状态更新
      emit sig_updateValveStatus((uint8_t)(data.at(0)),
                                 valveStatus); // 返回SN号和对应的阀门状态
    }
  }
  // 解析写入的返回值
  else if (data.at(1) == 0x06) {
  }
}

void DeviceInfoReset(DeviceInfo &device) {
  device.airPressUpdateFlag = false;
  device.infPressUpdateFlag = false;
  device.endPressUpdateFlag = false;
}
