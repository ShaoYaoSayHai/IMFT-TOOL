
#ifndef _MODBUS_WRITE_BUILD_H_
#define _MODBUS_WRITE_BUILD_H_

#include <QByteArray>
#include <QDebug>
#include <QString>

/**
 * @brief 向加糖通信协议写入
 * @param slaveId
 * @param regAddr
 * @param regVal
 * @param len
 * @return
 */
QByteArray BuildWriteGtDevice(uint8_t slaveId, uint16_t regAddr,
                              uint8_t *regVal, uint8_t len) ;

/**
 * @brief 向加糖通信协议读取
 * @param slaveAddress
 * @param regAddr
 * @return
 */
QByteArray BuildReadGtDevice(uint8_t slaveAddress, uint16_t regAddr);

/**
 * @brief 向Modbus设备写入数据
 * @param slaveAddr
 * @param regAddr
 * @param regVal
 * @param len
 * @return
 */
QByteArray BuildWriteModbusDevice(uint8_t slaveAddr, uint16_t regAddr,
                                  uint8_t *regVal, uint8_t len) ;

/**
 * @brief 向Modbus设备读取数据
 * @param slaveAddr
 * @param regAddr
 * @param len
 * @return
 */
QByteArray BuildReadModbusDevice(uint8_t slaveAddr, uint16_t regAddr,
                                 uint8_t len) ;


#endif
