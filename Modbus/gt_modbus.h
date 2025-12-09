// SPDX-License-Identifier: LGPL-3.0-only
// 许可声明：本文件依据 GNU LGPL v3 许可条款分发与使用。
// 完整许可文本见项目根目录：LPGL3.md

#ifndef GT_MODBUS_H
#define GT_MODBUS_H

#include "ModbusReMapping.h"
#include "ModbusWriteBuild.h"
#include <QMetaType> // 确保包含这个头文件
#include <QObject>

typedef struct DeviceInfo {
  QByteArray SN;
  QByteArray slaveID;
  uint32_t airPress;
  uint32_t infPress;
  uint32_t endPress;
  int Status;
  int airPressUpdateFlag;
  int infPressUpdateFlag;
  int endPressUpdateFlag;

  bool sw_status ;
  bool low_press_status ;
  bool over_press_status ;

} DeviceInfo;

typedef struct DoTestType {
  bool ChaoYaTest;
  bool QianYaTest;
} DoTestType;

#define REG_GT_AIRP (0x4001)
#define REG_GT_FAQIAN (0x4010)
#define REG_GT_FAHOU (0x4012)
#define REG_GT_SWITCH (0x4033)
#define REG_GT_READSN (0x4045)
#define REG_GT_FACTORY (0x4047)
#define REG_GT_CLEAR_ERR (0x5080)
#define REG_GT_SET_SW_MODE (0x5084)

// 声明元类型
Q_DECLARE_METATYPE(DeviceInfo)

void DeviceInfoReset(DeviceInfo &device);

class GT_Modbus : public QObject {
  Q_OBJECT
public:
  explicit GT_Modbus(QObject *parent = nullptr);
  ~GT_Modbus();

  enum GT_MODBUS {
    SUCCESS,
    ERROR,
    TIMEOUT,
  };

  Q_ENUM(GT_MODBUS);

  enum ModbusMode {
    MODBUS_GT,
    MODBUS_RTU,
  };
  Q_ENUM(ModbusMode);

  enum AirPressureLocation {
    AIR, // 大气压
    INF, // 进气端
    END, // 出气端
  };
  Q_ENUM(AirPressureLocation);

  QByteArray GT_ModbusWrite(uint8_t slaveAddr, uint8_t funcCode,
                            uint16_t regAddr, uint8_t *regValue,
                            uint8_t regSize, ModbusReMappingFunc func);

  // 返回的值进行校验
  int GT_RetMsgVerify(QByteArray data);

  // 设置从机地址
  void GT_SetSlaveAddr(uint8_t slaveID);

  // 获取从机地址
  uint8_t GT_GetSlaveAddr();

private:
  uint8_t slaveAddr = 0x01; // 外设地址 默认0x01
  uint8_t funcCode;         // 指令
  uint16_t regAddr;         // 寄存器地址

  QByteArray qbyData;

signals:

  // 更新序列号
  void sig_updateSN(QByteArray sn);
  // 更新大气压值
  void sig_updateAirPressure(int sn, int location, int value);
  // 更新阀门状态
  void sig_updateValveStatus(int sn, int value);

  // 返回接收信息
  void GT_ReadInfo( QByteArray data );
};

#endif // GT_MODBUS_H
