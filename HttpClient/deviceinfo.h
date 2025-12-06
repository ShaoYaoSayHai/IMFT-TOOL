#pragma once

#include <QString>
#include <QVariantMap>
#include <QMap>

class DeviceInfo
{
public:
    DeviceInfo() = default;

    void fromAttributeMap(const QMap<QString, QString> &attrs);

    QString value(const QString &key) const;
    QVariantMap allValues() const;

    QString retVal;
    QString retMsg;
    QString model;
    QString wo;
    QString sn;
    QString imei1;
    QString imei2;
    QString mac1;
    QString mac2;
    QString pwd;
    QString acs;
    QString iccid;
    QString hotName;
    QString hotPwd;
    QString devKey;
    QString nal;
    QString hotName5;
    QString hotPwd5;
    QString moduleVer;
    QString routerVer;
    QString orderQuantity;

private:
    QVariantMap m_extras;
};
