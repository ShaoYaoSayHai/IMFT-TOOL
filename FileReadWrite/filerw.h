#ifndef FILERW_H
#define FILERW_H

#include <QObject>

#include <QDomDocument>
#include <QFile>
#include <QDebug>

// 定义设备信息结构体
struct CLTDeviceInfo {
    QString address;
    QString type;

    // 可选：添加构造函数方便初始化
    CLTDeviceInfo() = default;
    CLTDeviceInfo(const QString& addr, const QString& t) : address(addr), type(t) {}
};

/**
 * @brief readXmlWithDOM
 * @param filePath
 */
QList<CLTDeviceInfo> readXmlToStruct(QString filePath) ;

#endif // FILERW_H
