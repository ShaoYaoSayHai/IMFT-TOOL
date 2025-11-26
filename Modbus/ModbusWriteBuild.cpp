
#include "ModbusWriteBuild.h"
#include "ModbusReMapping.h"

/**
 * @brief 向加糖通信协议写入
 * @param slaveId
 * @param regAddr
 * @param regVal
 * @param len
 * @return
 */
QByteArray BuildWriteGtDevice(uint8_t slaveId, uint16_t regAddr,
                              uint8_t *regVal, uint8_t len)
{
    QByteArray ret;
    ret.append((uint8_t)slaveId);
    ret.append((uint8_t)0x06);
    ret.append((uint8_t)(regAddr >> 8));
    ret.append((uint8_t)(regAddr & 0xFF));
    ret.append((uint8_t)len); // 写入数据长度
    for (int i = 0; i < len; i++) {
        ret.append((uint8_t)(regVal[i] >> 8));
        ret.append((uint8_t)(regVal[i] & 0xFF));
    }
    uint16_t crc = crc16_MODBUS(ret);
    ret.append((uint8_t)(crc & 0xFF));
    ret.append((uint8_t)(crc >> 8));
    return ret;
}

/**
 * @brief 向加糖通信协议读取
 * @param slaveAddress
 * @param regAddr
 * @return
 */
QByteArray BuildReadGtDevice(uint8_t slaveAddress, uint16_t regAddr) {
    QByteArray ret;
    ret.append(slaveAddress);
    ret.append(0x03);
    ret.append((uint8_t)(regAddr >> 8));
    ret.append((uint8_t)(regAddr & 0xFF));
    ret.append((uint8_t)(0x00));
    uint16_t crc = crc16_MODBUS(ret);
    ret.append((uint8_t)(crc & 0xFF));
    ret.append((uint8_t)(crc >> 8));
    return ret;
}

/**
 * @brief 向Modbus设备写入数据
 * @param slaveAddr
 * @param regAddr
 * @param regVal
 * @param len
 * @return
 */
QByteArray BuildWriteModbusDevice(uint8_t slaveAddr, uint16_t regAddr,
                                  uint8_t *regVal, uint8_t len) {
    QByteArray ret;
    ret.append((uint8_t)slaveAddr);
    ret.append((uint8_t)0x10);
    ret.append((uint8_t)(regAddr >> 8));
    ret.append((uint8_t)(regAddr & 0xFF));
    ret.append((uint8_t)(len >> 8));
    ret.append((uint8_t)(len & 0xFF));
    ret.append((uint8_t)len * 2); // 写入数据长度
    for (int i = 0; i < len; i++) {
        ret.append((uint8_t)(regVal[i] >> 8));
        ret.append((uint8_t)(regVal[i] & 0xFF));
    }
    uint16_t crc = crc16_MODBUS(ret);
    ret.append((uint8_t)(crc & 0xFF));
    ret.append((uint8_t)(crc >> 8));
    return ret;
}

/**
 * @brief 向Modbus设备读取数据
 * @param slaveAddr
 * @param regAddr
 * @param len
 * @return
 */
QByteArray BuildReadModbusDevice(uint8_t slaveAddr, uint16_t regAddr,
                                 uint8_t len) {
    QByteArray ret;
    ret.append((uint8_t)slaveAddr);
    ret.append((uint8_t)0x03);
    ret.append((uint8_t)(regAddr >> 8));
    ret.append((uint8_t)(regAddr & 0xFF));
    ret.append((uint8_t)(len >> 8));
    ret.append((uint8_t)(len & 0xFF));
    uint16_t crc = crc16_MODBUS(ret);
    ret.append((uint8_t)(crc & 0xFF));
    ret.append((uint8_t)(crc >> 8));
    return ret;
}
