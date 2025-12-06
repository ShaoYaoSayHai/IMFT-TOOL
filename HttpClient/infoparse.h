#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>

class DeviceInfo;

class InfoParser : public QObject
{
    Q_OBJECT
public:
    explicit InfoParser(QObject *parent = nullptr);

    bool parseJsonPayload(const QByteArray &json, QString *xmlOut, QString *error) const;
    bool parseInfoXml(const QString &xml, DeviceInfo *info, QString *error) const;
    static QMap<QString , QString> JsonPayloadParse(QByteArray &jsonPayload);

    // 解析XML并获取SN和STA的值
    static bool parseXml(const QString& xmlString, QString& snValue, QString& staValue) ;
    // 替换SN和STA的值
    static QString replaceAttributes(const QString& xmlString,
                                     const QString& newSN = QString(),
                                     const QString& newSTA = QString()) ;

    static QString generateXmlString(const QString& sn, const QString& sta) ;
};

QString buildInputPayload(const QString &sn,
                          const QString &sta,
                          const QString &imei1,
                          const QString &iccid) ;
