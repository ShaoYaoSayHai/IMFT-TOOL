#pragma once

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <QDebug>
#include <QRegularExpression>

struct ParseResult
{
    bool ok = false;        // true => PASS；false => FAIL
    QString sn;             // 提取到的 SN（可能为空）
    QString status;         // "PASS" or "FAIL"
    QString retmsg;         // XML里的RETMSG（如有）
    QString rawError;       // 非XML场景下的原始错误文本（如有）
};

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


bool parseRetmsgPassFromJson(const QString& jsonString, QString* outRetmsg) ;

QByteArray buildInputXmlBytes(const QString& sn,
                              const QString& sta,
                              const QString& result /* PASS/FAIL */) ;


bool extractXmlFromServerResp(const QByteArray& respJsonBytes,
                              QString* outXml,
                              QString* outErr) ;

// 解析MES CHECK的返回值
ParseResult parseSnAndStatus(const QString& input) ;
