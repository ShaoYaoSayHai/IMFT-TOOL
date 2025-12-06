#ifndef FILERW_H
#define FILERW_H

#include <QObject>

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QTextStream>

// 定义设备信息结构体
struct CLTDeviceInfo {
  QString address;
  QString type;

  // 可选：添加构造函数方便初始化
  CLTDeviceInfo() = default;
  CLTDeviceInfo(const QString &addr, const QString &t)
      : address(addr), type(t) {}
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

#endif // FILERW_H
