#ifndef BASESERIALPORT_H
#define BASESERIALPORT_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>


class BaseSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit BaseSerialPort(QObject *parent = nullptr);

    ~BaseSerialPort();

    enum ConnectStatus{
        Error ,
        Connected ,
        UnConnected ,
    };
    // 注册成QT独有的Enum
    Q_ENUM(ConnectStatus) ;

    bool getQSerialPortPtr();

private:

    // 串口指针
    QSerialPort  *pxSerialPort = nullptr ;
    // 是否连接的判断值
    bool bsp_connected = false ;

public slots:
    // 公开接口
    /* 串口初始化 */
    void SerialPortInit();
    /* 串口连接 */
    void SerialPortConnect( QString portName , int baudRate );
    /* 串口发送数据 */
//    void SerialPortWrite( QByteArray data );
    void SerialPortWrite( char *buffer , int len );

    /* 串口读取 */
    void SerialPortReadAll();

    /* 断开连接 */
    void SerialPortDisconnect();


signals:

    void ret_connect_status( int status );

    void ret_read_data( QByteArray data );

private slots:
    void SerialPortErrorHappend(QSerialPort::SerialPortError error);
};

#endif // BASESERIALPORT_H
