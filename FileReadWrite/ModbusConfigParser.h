#ifndef MODBUSCONFIGPARSER_H
#define MODBUSCONFIGPARSER_H

#include <QList>
#include <QByteArray>
#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QMap>
#include <QRegularExpression>
#include <QDebug>

/**
 * @brief 任务信息结构体
 * 表示一个Modbus通信任务的所有参数
 */
struct TaskInfo {
    int id;                         ///< 配置ID
    bool enable;                    ///< 是否启用
    QList<uint8_t> slave_ids;       ///< 从机地址列表（多个地址）
    uint8_t func;                   ///< 功能码（如0x03读、0x06写）
    uint16_t address;               ///< 寄存器地址
    QList<QByteArray> write_buffers; ///< 写入缓冲区列表（按step排序的缓冲区列表）
    int read_length;                ///< 读取长度（0表示写操作）
    uint16_t time_interval ;        ///<读取时间间隔

    // 默认构造函数
    TaskInfo() : id(0), enable(false), func(0), address(0), read_length(0) {}

    // 打印任务信息（用于调试）
    void printInfo() const;
};

/**
 * @brief Modbus配置解析器类
 * 用于解析XML格式的Modbus命令配置
 */
class ModbusConfigParser
{
public:
    /**
     * @brief 从文件路径解析XML配置文件，生成任务列表
     *
     * @param filePath XML配置文件路径
     * @param gtSlaveIds 当slave_id的type="GT"时使用的从机地址列表
     * @return QList<TaskInfo> 解析后的任务列表
     */
    static QList<TaskInfo> parseConfig(const QString& filePath, const QList<uint8_t>& gtSlaveIds);

    /**
     * @brief 从XML字符串解析配置，生成任务列表
     *
     * @param xmlContent XML配置内容字符串
     * @param gtSlaveIds 当slave_id的type="GT"时使用的从机地址列表
     * @return QList<TaskInfo> 解析后的任务列表
     */
    static QList<TaskInfo> parseConfigFromString(const QString& xmlContent, const QList<uint8_t>& gtSlaveIds);

public slots:
//    QList<TaskInfo> parseConfigWithCommandName(const QDomDocument &doc, const QList<uint8_t> &gtSlaveIds, QString rootNodeListName, QString nodeName);
    static QList<TaskInfo> parseConfigWithCommandName(const QString &filePath, const QList<uint8_t> &gtSlaveIds, QString rootNodeListName, QString nodeName);
private slots:
    static QList<TaskInfo> parseConfigImplWithCommandName(const QDomDocument &doc, const QList<uint8_t> &gtSlaveIds, QString rootNodeListName, QString nodeName);
private:
    /**
     * @brief 解析十六进制字符串到字节数组
     *
     * @param hexString 十六进制字符串（支持逗号、空格、斜杠分隔）
     * @return QByteArray 解析后的字节数组
     */
    static QByteArray parseHexString(const QString& hexString);

    /**
     * @brief 解析从机地址列表
     *
     * @param addrString 地址字符串（逗号分隔的十六进制地址）
     * @return QList<uint8_t> 从机地址列表
     */
    static QList<uint8_t> parseSlaveIds(const QString& addrString);

    /**
     * @brief 实际解析配置的核心函数
     *
     * @param doc 已加载的XML文档对象
     * @param gtSlaveIds 当slave_id的type="GT"时使用的从机地址列表
     * @return QList<TaskInfo> 解析后的任务列表
     */
    static QList<TaskInfo> parseConfigImpl(const QDomDocument& doc, const QList<uint8_t>& gtSlaveIds);
};

#endif // MODBUSCONFIGPARSER_H
