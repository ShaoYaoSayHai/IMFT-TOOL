#include "deviceinfo.h"

void DeviceInfo::fromAttributeMap(const QMap<QString, QString> &attrs)
{
    auto fetch = [&](const QString &key) -> QString {
        auto it = attrs.find(key);
        return it == attrs.end() ? QString() : it.value();
    };

    retVal        = fetch(QStringLiteral("RETVAL"));
    retMsg        = fetch(QStringLiteral("RETMSG"));
    model         = fetch(QStringLiteral("MODEL"));
    wo            = fetch(QStringLiteral("WO"));
    sn            = fetch(QStringLiteral("SN"));
    imei1         = fetch(QStringLiteral("IMEI1"));
    imei2         = fetch(QStringLiteral("IMEI2"));
    mac1          = fetch(QStringLiteral("MAC1"));
    mac2          = fetch(QStringLiteral("MAC2"));
    pwd           = fetch(QStringLiteral("PWD"));
    acs           = fetch(QStringLiteral("ACS"));
    iccid         = fetch(QStringLiteral("ICCID"));
    hotName       = fetch(QStringLiteral("HOTNAME"));
    hotPwd        = fetch(QStringLiteral("HOTPWD"));
    devKey        = fetch(QStringLiteral("DEV_KEY"));
    nal           = fetch(QStringLiteral("NAL"));
    hotName5      = fetch(QStringLiteral("HOTNAME5"));
    hotPwd5       = fetch(QStringLiteral("HOTPWD5"));
    moduleVer     = fetch(QStringLiteral("MODULEVER"));
    routerVer     = fetch(QStringLiteral("ROUTERVER"));
    orderQuantity = fetch(QStringLiteral("ORDERQUANTITY"));

    m_extras.clear();
    for (auto it = attrs.cbegin(); it != attrs.cend(); ++it) {
        m_extras.insert(it.key(), it.value());
    }
}

QString DeviceInfo::value(const QString &key) const
{
    return m_extras.value(key).toString();
}

QVariantMap DeviceInfo::allValues() const
{
    return m_extras;
}
