#ifndef FILERW_H
#define FILERW_H

#include <QObject>

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>

// 定义设备信息结构体
struct CLTDeviceInfo {
  QString address;
  QString type;

  // 可选：添加构造函数方便初始化
  CLTDeviceInfo() = default;
  CLTDeviceInfo(const QString &addr, const QString &t)
      : address(addr), type(t) {}
};

struct CommandParams {
    int id = 0;
    bool enable = false;
    QList<quint8> slave_id;  // 使用QList存储从机ID
    quint16 func = 0;
    quint32 address = 0;
    QByteArray write_buffer;
    int read_length = 0;
    QString commandType;  // 命令类型标识
};

/**
 * @brief readXmlWithDOM
 * @param filePath
 */
QList<CLTDeviceInfo> readXmlToStruct(QString filePath);

bool readSwitchTimesWithDebug(const QString &filePath, int &switchOpenTime,
                              int &switchCloseTime, int &switchResetTime);

bool readPressureTimeConfig(const QString &filePath, int &readAirPressureTime,
                            int &readFaqianPressureTime,
                            int &readFahouPressureTime);

bool readInputControlSwitch(const QString &filePath, uint8_t &a1Addr,
                            uint8_t &a2Addr);

bool readPressureTimeoutConfig(const QString &filePath, int &readTimeout1,
                               int &readTimeout2);

/**
 * @brief readInternetMesConfigInfo 读取配置信息
 * @param filePath 路径
 * @param part 目标内容
 * @return
 */
QString readInternetMesConfigInfo(const QString &filePath , QString part ) ;

bool FileTakeRootElement(const QString& path, QDomElement& rootElement, QString& errorMsg, int* errorLine = 0, int* errorColumn = 0) ;

bool read_xml_cmd( QString path ) ;

// 解析slave_id的辅助函数
QList<quint8> parseSlaveId(const QString& text, const QDomElement& element) ;

// 解析write_buffer的辅助函数
QByteArray parseWriteBuffer(const QString& text, const QDomElement &element) ;

// 核心解析函数 - 解析单个命令元素
bool parseCommandElement(const QDomElement& element, CommandParams& params) ;

// 主解析函数 - 遍历所有命令并排序
QList<CommandParams> parseAllCommands(const QDomElement& rootElement) ;

// 使用示例：从XML文件解析
QList<CommandParams> parseXmlFile(const QString& filePath) ;

QString parseLoginResponse(const QString& jsonResponse) ;

bool parseStaIsOmft(const QString& xml, QString* outSta = nullptr) ;

#endif // FILERW_H
