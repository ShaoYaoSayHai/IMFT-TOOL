#include "infoparse.h"
#include "deviceinfo.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QXmlStreamReader>
#include <QMap>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QBuffer>
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

QByteArray buildInputXmlBytes(const QString& sn,
                              const QString& sta,
                              const QString& result /* PASS/FAIL */)
{
    QByteArray xmlBytes;
    QBuffer buf(&xmlBytes);
    buf.open(QIODevice::WriteOnly);

    QXmlStreamWriter xw(&buf);
    xw.setAutoFormatting(false);

    xw.writeStartElement("root");
    xw.writeEmptyElement("info");

    xw.writeAttribute("SN", sn);
    xw.writeAttribute("STA", sta);
    xw.writeAttribute("RESULT", result);

    xw.writeEndElement(); // root
    return xmlBytes;
}


bool extractXmlFromServerResp(const QByteArray& respJsonBytes,
                              QString* outXml,
                              QString* outErr)
{
    if (outXml) outXml->clear();
    if (outErr) outErr->clear();

    QJsonParseError pe{};
    const QJsonDocument doc = QJsonDocument::fromJson(respJsonBytes, &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        if (outErr) *outErr = QStringLiteral("JSON parse failed: %1").arg(pe.errorString());
        return false;
    }

    const QJsonObject obj = doc.object();
    const QJsonValue v = obj.value(QStringLiteral("d"));
    if (!v.isString()) {
        if (outErr) *outErr = QStringLiteral("Field 'd' missing or not a string.");
        return false;
    }

    // 关键点：这里拿到的是“已解码的真实字符串”，\u003c 会变成 <，\" 会变成 "
    const QString xml = v.toString();
    if (outXml) *outXml = xml;
    return true;
}




ParseResult parseSnAndStatus(const QString& input)
{
    ParseResult r;
    const QString s = input.trimmed();

    // ========= 1) 尝试从任意位置提取 XML 段 =========
    // 你的成功字符串前面有 "MES PASS ::" 前缀，所以不能用 startsWith('<')
    int xmlPos = s.indexOf(QStringLiteral("<root"));
    if (xmlPos < 0) {
        xmlPos = s.indexOf(QStringLiteral("<info")); // 兜底：有时可能没有 root
    }

    if (xmlPos >= 0) {
        const QString xml = s.mid(xmlPos).trimmed();

        QXmlStreamReader xr(xml);
        while (!xr.atEnd()) {
            xr.readNext();
            if (xr.isStartElement() && xr.name() == QStringLiteral("info")) {
                const auto attrs = xr.attributes();

                r.sn = attrs.value(QStringLiteral("SN")).toString();
                r.retmsg = attrs.value(QStringLiteral("RETMSG")).toString();

                // 规则：仅 RETMSG == PASS 才成功，否则一律 FAIL
                r.ok = (r.retmsg.compare(QStringLiteral("PASS"), Qt::CaseInsensitive) == 0);
                r.status = r.ok ? QStringLiteral("PASS") : QStringLiteral("FAIL");
                return r;
            }
        }

        // XML 存在但解析失败/没找到 info：按失败处理
        r.ok = false;
        r.status = QStringLiteral("FAIL");
        r.rawError = xr.hasError() ? xr.errorString()
                                   : QStringLiteral("XML present but missing <info> element.");
        return r;
    }

    // ========= 2) 非 XML：解析失败文本中的 SN =========
    // 适配："[time] SN:CE02_...xxx" / "SN=..." / "SN：..."
    static const QRegularExpression reSn(R"(SN\s*[:=：]\s*([A-Za-z0-9_]+))");
    const QRegularExpressionMatch m = reSn.match(s);
    if (m.hasMatch()) {
        r.sn = m.captured(1);
    }

    r.ok = false;
    r.status = QStringLiteral("FAIL");
    r.rawError = s;
    return r;
}




static bool extractXmlSegment(const QString& s, QString* outXml)
{
    if (!outXml) return false;
    outXml->clear();

    // 1) 优先截取 <root ... </root>
    int start = s.indexOf(QLatin1String("<root"));
    if (start >= 0) {
        int end = s.indexOf(QLatin1String("</root>"), start);
        if (end >= 0) {
            end += int(strlen("</root>"));
            *outXml = s.mid(start, end - start).trimmed();
            return !outXml->isEmpty();
        }
        // 只有 <root ... 但无 </root>，不返回，继续走 <info> 兜底
    }

    // 2) 兜底截取 <info .../>
    start = s.indexOf(QLatin1String("<info"));
    if (start >= 0) {
        int end = s.indexOf(QLatin1String("/>"), start);
        if (end >= 0) {
            end += 2;
            *outXml = s.mid(start, end - start).trimmed();
            return !outXml->isEmpty();
        }
        // 或 <info> ... </info>
        end = s.indexOf(QLatin1String("</info>"), start);
        if (end >= 0) {
            end += int(strlen("</info>"));
            *outXml = s.mid(start, end - start).trimmed();
            return !outXml->isEmpty();
        }
    }

    return false;
}

ParseResult parseSnAndRetmsg_StrongCompat(const QString& input)
{
    ParseResult r;
    const QString s = input.trimmed();

    // ---------- 1) 先截取 XML 片段 ----------
    QString xml;
    if (extractXmlSegment(s, &xml)) {
        QXmlStreamReader xr(xml);

        while (!xr.atEnd()) {
            xr.readNext();

            if (xr.isStartElement() &&
                xr.name().toString().compare(QLatin1String("info"), Qt::CaseInsensitive) == 0) {

                const QXmlStreamAttributes attrs = xr.attributes();

                // 关键：遍历属性，做大小写不敏感匹配（比 attrs.value("SN") 更稳）
                for (const QXmlStreamAttribute& a : attrs) {
                    const QString key = a.name().toString();
                    if (key.compare(QLatin1String("SN"), Qt::CaseInsensitive) == 0) {
                        r.sn = a.value().toString();
                    } else if (key.compare(QLatin1String("RETMSG"), Qt::CaseInsensitive) == 0) {
                        r.retmsg = a.value().toString();
                    }
                }

                r.ok = (r.retmsg.compare(QLatin1String("PASS"), Qt::CaseInsensitive) == 0);
                r.status = r.ok ? QStringLiteral("PASS") : QStringLiteral("FAIL");
                return r; // 找到 <info> 就返回
            }
        }

        // XML 有但解析失败/没找到 info
        r.rawError = xr.hasError()
                   ? xr.errorString()
                   : QStringLiteral("XML present but missing <info> element.");
        // 不直接 return，继续走正则兜底
    }

    // ---------- 2) 正则兜底：从整段文本中提取 SN/RETMSG ----------
    {
        QRegularExpression reSn(QStringLiteral("SN\\s*=\\s*\"([^\"]+)\""));
        QRegularExpressionMatch m = reSn.match(s);
        if (m.hasMatch())
            r.sn = m.captured(1).trimmed();
    }
    {
        QRegularExpression reMsg(QStringLiteral("RETMSG\\s*=\\s*\"([^\"]+)\""));
        QRegularExpressionMatch m = reMsg.match(s);
        if (m.hasMatch())
            r.retmsg = m.captured(1).trimmed();
    }

    r.ok = (r.retmsg.compare(QLatin1String("PASS"), Qt::CaseInsensitive) == 0);
    r.status = r.ok ? QStringLiteral("PASS") : QStringLiteral("FAIL");

    if (r.sn.isEmpty() || r.retmsg.isEmpty()) {
        if (r.rawError.isEmpty())
            r.rawError = QStringLiteral("SN/RETMSG not found.");
    }

    return r;
}


