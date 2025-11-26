#ifndef _MODBUS_RE_MAPPING_H_
#define _MODBUS_RE_MAPPING_H_

#include <QByteArray>

typedef int (*ModbusReMappingFunc)(QByteArray &data);

// 初始化
extern ModbusReMappingFunc xModbsuFunc;

/**
 * @brief 计算数据的 CRC-16 MODBUS 校验值
 * @param puc_data 指向输入数据缓冲区的指针
 * @param us_length 输入数据的长度（字节数）
 * @return 计算得到的 CRC-16 MODBUS 校验值
 */
uint16_t crc16_MODBUS(uint8_t *puc_data, uint16_t us_length);

/**
 * @brief 计算QByteArray数据的Modbus CRC16校验值
 * @param byteArray 包含待校验数据的QByteArray对象
 * @return 计算得到的CRC16校验值（16位无符号整数）
 *
 * @note MODBUS协议要求校验值在传输时低字节在前
 * 例如：uint8_t crc_low = (uint8_t)(crc & 0xFF);
 *       uint8_t crc_high = (uint8_t)(crc >> 8);
 * 然后将crc_low放在帧中先传输，crc_high随后[1,3](@ref)
 */
uint16_t crc16_MODBUS(const QByteArray &byteArray);

int ParseModbusPressurePointRead(QByteArray &data);

int ParseModbusPressureValueRead(QByteArray &data);

int ParseModbusPressureInTotal(QByteArray &data);

/**
 * @brief ParseGtDeviceSlaveID 读取加糖设备的信息
 * @param data
 * @return
 */
QByteArray ParseGtDeviceReturnMsg(QByteArray &data) ;

/**
 * @brief 解析Modbus GT-Air压力传感器读取数据
 * @param data 包含Modbus GT-Air压力传感器读取数据的QByteArray对象
 * @return 解析得到的压力值（单位：Pa）
 */
int ParseGtAirPressureRead(QByteArray &data)  ;

#endif // _MODBUS_RE_MAPPING_H_
