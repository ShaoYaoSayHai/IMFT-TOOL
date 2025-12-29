#include "infoparse.h"
#include "deviceinfo.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <QMap>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QDebug>

InfoParser::InfoParser(QObject *parent)
    : QObject(parent)
{
}

bool InfoParser::parseJsonPayload(const QByteArray &json, QString *xmlOut, QString *error) const
{
    const QJsonDocument doc = QJsonDocument::fromJson(json);
    if (doc.isNull() || !doc.isObject()) {
        if (error) *error = QStringLiteral("JSON 解析失败：文档为空或不是对象。");
        return false;
    }

    const QJsonObject obj = doc.object();
    if (!obj.contains(QStringLiteral("d")) || !obj.value(QStringLiteral("d")).isString()) {
        if (error) *error = QStringLiteral("JSON 中缺少字符串字段 \"d\"。");
        return false;
    }

    if (xmlOut) {
        *xmlOut = obj.value(QStringLiteral("d")).toString();
    }
    return true;
}

bool InfoParser::parseInfoXml(const QString &xml, DeviceInfo *info, QString *error) const
{
    if (!info) {
        if (error) *error = QStringLiteral("DeviceInfo 指针为空。");
        return false;
    }

    QXmlStreamReader reader(xml);
    QMap<QString, QString> attributes;

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::StartElement &&
                reader.name() == QStringLiteral("info")) {

            const auto attrs = reader.attributes();
            for (const auto &attr : attrs) {
                attributes.insert(attr.name().toString(), attr.value().toString());
            }
            break;
        }
    }

    if (reader.hasError()) {
        if (error) *error = QStringLiteral("XML 解析错误：%1").arg(reader.errorString());
        return false;
    }
    if (attributes.isEmpty()) {
        if (error) *error = QStringLiteral("未找到 <info> 节点或其属性。");
        return false;
    }

    info->fromAttributeMap(attributes);
    return true;
}

QMap<QString , QString> InfoParser::JsonPayloadParse( QByteArray &jsonPayload )
{
    //    qDebug()<<"JsonPayloadParse - "<<jsonPayload ;
    InfoParser parser;
    QString xml;
    QString error;
    if (!parser.parseJsonPayload(jsonPayload, &xml, &error)) {
        qWarning() << "JSON 解析失败:" << error;
        return {};
    }
    DeviceInfo info;
    if (!parser.parseInfoXml(xml, &info, &error)) {
        qWarning() << "XML 解析失败:" << error;
        return {};
    }
    QString SN = info.value(QStringLiteral("SN")) ;
    QString ICCID = info.value(QStringLiteral("ICCID")) ;
    QString IMEI1 = info.value(QStringLiteral("IMEI1")) ;

    if( !SN.isEmpty() && !ICCID.isEmpty() && !IMEI1.isEmpty() )
    {
        QMap<QString , QString> map ;
        map.insert( "SN" , info.value(QStringLiteral("SN")) );
        map.insert( "ICCID" , info.value(QStringLiteral("ICCID")) );
        map.insert( "IMEI1" , info.value(QStringLiteral("IMEI1")) );
        return map ;
    }
    //    const QVariantMap all = info.allValues();
    //    for (auto it = all.cbegin(); it != all.cend(); ++it) {
    //        qDebug().nospace() << it.key() << " = " << it.value().toString();
    //    }
    return {} ;
}

QString buildInputPayload(const QString &sn,
                          const QString &sta,
                          const QString &imei1,
                          const QString &iccid)
{
    const QString xml = QStringLiteral(
                "<root><info SN=\"%1\" STA=\"%2\" IMEI1=\"%3\" PWD=\"\" RESULT=\"PASS\" "
                "FAILMSG=\"+\" ROUTERINFO=\"\" MODEMINFO=\"\" SN1=\"\" SN2=\"\" "
                "ICCID2=\"\" ICCID=\"%4\" /></root>")
            .arg(sn.toHtmlEscaped(),
                 sta.toHtmlEscaped(),
                 imei1.toHtmlEscaped(),
                 iccid.toHtmlEscaped());

    return xml ;
}



// 解析XML并获取SN和STA的值
bool parseXml(const QString& xmlString, QString& snValue, QString& staValue) {
    QDomDocument doc;
    if (!doc.setContent(xmlString)) {
        qWarning() << "Failed to parse XML string";
        return false;
    }

    QDomElement root = doc.documentElement();
    QDomElement infoElement = root.firstChildElement("info");

    if (infoElement.isNull()) {
        qWarning() << "Info element not found";
        return false;
    }

    snValue = infoElement.attribute("SN");
    staValue = infoElement.attribute("STA");

    return !snValue.isEmpty() && !staValue.isEmpty();
}

// 替换SN和STA的值
QString replaceAttributes(const QString& xmlString,
                          const QString& newSN = QString(),
                          const QString& newSTA = QString()) {
    QDomDocument doc;
    if (!doc.setContent(xmlString)) {
        qWarning() << "Failed to parse XML string";
        return QString();
    }

    QDomElement root = doc.documentElement();
    QDomElement infoElement = root.firstChildElement("info");

    if (infoElement.isNull()) {
        qWarning() << "Info element not found";
        return QString();
    }

    // 替换属性值（如果提供了新值）
    if (!newSN.isEmpty()) {
        infoElement.setAttribute("SN", newSN);
    }
    if (!newSTA.isEmpty()) {
        infoElement.setAttribute("STA", newSTA);
    }

    return doc.toString();
}


QString InfoParser::generateXmlString(const QString& sn, const QString& sta) {
    QString xmlString;
    QXmlStreamWriter writer(&xmlString);

    // 设置不自动格式化，以保持紧凑格式
    writer.setAutoFormatting(false);

    // 开始写入XML
    //    writer.writeStartDocument();
    writer.writeStartElement("root");
    writer.writeStartElement("info");

    // 设置SN和STA属性
    writer.writeAttribute("SN", sn);
    writer.writeAttribute("STA", sta);

    // 结束元素
    writer.writeEndElement(); // 结束info元素
    writer.writeEndElement(); // 结束root元素
    //    writer.writeEndDocument();

    return xmlString;
}




bool parseRetmsgPassFromJson(const QString& jsonString, QString* outRetmsg)
{
    if (outRetmsg) outRetmsg->clear();

    // 1) 解析 JSON
    QJsonParseError jerr;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &jerr);
    if (jerr.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[parseRetmsgPassFromJson] JSON parse error:" << jerr.errorString();
        return false;
    }

    const QJsonObject obj = doc.object();
    const QJsonValue dVal = obj.value(QStringLiteral("d"));
    if (!dVal.isString()) {
        qWarning() << "[parseRetmsgPassFromJson] JSON field 'd' missing or not a string";
        return false;
    }

    const QString xml = dVal.toString();

    // 2) 解析 XML，找到 <info> 的 RETMSG 属性
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement() && reader.name() == QLatin1String("info")) {
            const QString retmsg = reader.attributes().value(QLatin1String("RETMSG")).toString().trimmed();

            if (outRetmsg) *outRetmsg = retmsg;

            // 3) 判断 PASS
            return (retmsg == QLatin1String("PASS"));
        }
    }

    if (reader.hasError()) {
        qWarning() << "[parseRetmsgPassFromJson] XML parse error:" << reader.errorString();
    } else {
        qWarning() << "[parseRetmsgPassFromJson] <info> element not found in XML";
    }
    return false;
}


