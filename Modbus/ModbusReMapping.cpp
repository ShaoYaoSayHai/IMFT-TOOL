
#include "ModbusReMapping.h"
#include <QByteArray>
#include <QDebug>
#include <hexprintf.h>

// 初始化
ModbusReMappingFunc xModbsuFunc;

/**
 * @brief 计算数据的 CRC-16 MODBUS 校验值
 * @param puc_data 指向输入数据缓冲区的指针
 * @param us_length 输入数据的长度（字节数）
 * @return 计算得到的 CRC-16 MODBUS 校验值
 */
uint16_t crc16_MODBUS(uint8_t *puc_data, uint16_t us_length) {
    uint16_t crc = 0xFFFF; // 初始化 CRC 寄存器为 0xFFFF
    for (uint16_t i = 0; i < us_length; i++) {
        crc ^= (uint16_t)puc_data[i]; // 当前数据字节与 CRC 进行异或
        // 处理每个字节的 8bytes
        for (uint8_t j = 0; j < 8; j++) {
            // 判断当前 CRC 的最低位 (LSB) 是否为为1
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // 右移一位并与反向多项式 0xA001 异或
            } else {
                crc >>= 1; // 仅右移一位
            }
        }
    }
    // 注意：MODBUS 协议要求校验值在传输时低字节在前
    // 此函数返回值本身是16位的整型，调用方需确保在组成通信帧时，将其转换为两个字节并以低字节在前（小端序）方式放置。
    // 例如：uint8_t crc_low = (uint8_t)(crc & 0xFF); uint8_t crc_high =
    // (uint8_t)(crc >> 8); 然后将 crc_low 放在帧中先传输，crc_high 随后。
    return crc;
}

uint16_t calculate_crc(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return (crc >> 8) | (crc << 8);;
}

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
uint16_t crc16_MODBUS(const QByteArray &byteArray) {
    if (byteArray.isEmpty()) {
        return 0xFFFF; // 空数据返回初始值
    }
    uint16_t crc = 0xFFFF; // 初始化CRC寄存器为0xFFFF
    // 获取数据指针和长度
    const char *data = byteArray.constData();
    int length = byteArray.size();
    for (int i = 0; i < length; i++) {
        crc ^= (uint16_t)((uint8_t)data[i]); // 当前数据字节与CRC进行异或
        // 处理每个字节的8个bit
        for (uint8_t j = 0; j < 8; j++) {
            // 判断当前CRC的最低位(LSB)是否为1
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001; // 右移一位并与反向多项式0xA001异或[3](@ref)
            } else {
                crc >>= 1; // 仅右移一位
            }
        }
    }
    return crc;
}

int ParseModbusPressurePointRead(QByteArray &data) {
    HexPrintf(data);
    return 0x0003;
}

int ParseModbusPressureValueRead(QByteArray &data) {
    HexPrintf(data);
    return 0x0004;
}

int ParseModbusPressureInTotal(QByteArray &data) {
    HexPrintf(data);
    return 0x0005;
}

/**
 * @brief 解析Modbus GT-Air压力传感器读取数据
 * @param data 包含Modbus GT-Air压力传感器读取数据的QByteArray对象
 * @return 解析得到的压力值（单位：Pa）
 */
int ParseGtAirPressureRead(QByteArray &data) {

    int value = 0;
    for( int i=0;i<3;i++ )
    {
        value = value << 8 ;
        value += (uint8_t)(data.at(i)) ;
    }
    return value;
}

QByteArray ParseGtDeviceReturnMsg(QByteArray &data)
{
    int size = data.at(4) ;
    QByteArray qbyData ;
    for(int i=0;i<size;i++)
    {
        qbyData.append(data.at(5+i)) ;
    }
    return  qbyData ; // 返回读取到的数据
}






