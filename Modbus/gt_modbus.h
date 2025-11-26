#ifndef GT_MODBUS_H
#define GT_MODBUS_H

#include <QObject>
#include "ModbusReMapping.h"
#include "ModbusWriteBuild.h"

class GT_Modbus : public QObject
{
    Q_OBJECT
public:
    explicit GT_Modbus(QObject *parent = nullptr);
    ~GT_Modbus();

    enum GT_MODBUS{
        SUCCESS ,
        ERROR ,
        TIMEOUT ,
    };

    Q_ENUM(GT_MODBUS) ;

    enum ModbusMode{
        MODBUS_GT ,
        MODBUS_RTU ,
    };
    Q_ENUM(ModbusMode) ;

    enum AirPressureLocation{
        AIR , // 大气压
        INF , // 进气端
        END , // 出气端
    };
    Q_ENUM(AirPressureLocation) ;

    QByteArray GT_ModbusWrite( uint8_t slaveAddr , uint8_t funcCode , uint16_t regAddr , uint8_t *regValue , uint8_t regSize , ModbusReMappingFunc func );

    // 返回的值进行校验
    int GT_RetMsgVerify( QByteArray data );

    // 设置从机地址
    void GT_SetSlaveAddr( uint8_t slaveID );

    // 获取从机地址
    uint8_t GT_GetSlaveAddr(  );

private:

    uint8_t slaveAddr = 0x01; // 外设地址 默认0x01
    uint8_t funcCode  ;// 指令
    uint16_t regAddr  ; // 寄存器地址

    QByteArray qbyData ;

signals:

    // 更新序列号
    void sig_updateSN( QByteArray sn );
    // 更新大气压值
    void sig_updateAirPressure( int location , int value );


};

#endif // GT_MODBUS_H
