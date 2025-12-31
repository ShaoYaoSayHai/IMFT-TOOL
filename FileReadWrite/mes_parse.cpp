#include "mes_parse.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QDebug>

static QString unescapeCommonJsonString(const QString& s)
{
    // 注意顺序：先处理 \\ 再处理 \" 可能会造成反复，这里按常见 JSON 反转义处理
    QString out = s;
    out.replace("\\\"", "\"");
    out.replace("\\n", "\n");
    out.replace("\\r", "\r");
    out.replace("\\t", "\t");
    out.replace("\\\\", "\\");
    return out;
}

static bool extractXmlByTagRange(const QString& input, QString* outXml)
{
    if (!outXml) return false;
    outXml->clear();

    // 优先截取 <root ... </root>
    const QString startTag = "<root";
    const QString endTag   = "</root>";

    int start = input.indexOf(startTag, 0, Qt::CaseInsensitive);
    if (start < 0) {
        // 容错：有些返回可能只含 <info .../>
        start = input.indexOf("<info", 0, Qt::CaseInsensitive);
        if (start < 0) return false;
    }

    int end = input.lastIndexOf(endTag, -1, Qt::CaseInsensitive);
    if (end >= 0) {
        end += endTag.length();
        *outXml = input.mid(start, end - start);
        return !outXml->isEmpty();
    }

    // 如果没有 </root>，尝试找 /> 作为结尾（仅当从 <info 起）
    int selfClose = input.indexOf("/>", start);
    if (selfClose >= 0) {
        selfClose += 2;
        *outXml = input.mid(start, selfClose - start);
        return !outXml->isEmpty();
    }

    return false;
}

static bool parseRetmsgFromXml(const QString& xml, QString* outRetmsg)
{
    if (outRetmsg) outRetmsg->clear();

    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement() && reader.name() == QLatin1String("info")) {
            const QString retmsg =
                reader.attributes().value(QLatin1String("RETMSG")).toString().trimmed();

            if (outRetmsg) *outRetmsg = retmsg;
            return !retmsg.isEmpty();
        }
    }

    // XML 不规范或解析失败时，做一次 regex 兜底（只要能抓到 RETMSG 就行）
    // 这对一些“半截 XML / 转义异常但仍可匹配”的情况非常有效
    {
        QRegularExpression re(QStringLiteral("RETMSG\\s*=\\s*\"([^\"]*)\""));
        QRegularExpressionMatch m = re.match(xml);
        if (m.hasMatch()) {
            const QString retmsg = m.captured(1).trimmed();
            if (outRetmsg) *outRetmsg = retmsg;
            return !retmsg.isEmpty();
        }
    }

    return false;
}

bool parseRetmsgPassFromJsonCompat(const QString& jsonString, QString* outRetmsg)
{
    if (outRetmsg) outRetmsg->clear();

    // ---------- 1) 先按“合法 JSON”路径解析 ----------
    {
        QJsonParseError jerr;
        const QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &jerr);

        if (jerr.error == QJsonParseError::NoError && doc.isObject()) {
            const QJsonObject obj = doc.object();
            const QJsonValue dVal = obj.value(QStringLiteral("d"));
            if (dVal.isString()) {
                const QString xml = dVal.toString(); // JSON 会自动把 \" 还原为 "
                QString retmsg;
                if (parseRetmsgFromXml(xml, &retmsg)) {
                    if (outRetmsg) *outRetmsg = retmsg;
                    return (retmsg == QLatin1String("PASS"));
                } else {
                    if (outRetmsg) *outRetmsg = QStringLiteral("XML_NO_RETMSG");
                    return false;
                }
            }

            // d 不是 string，继续走容错
        } else {
            // 解析失败（例如 unterminated object），继续走容错
            // 建议保留日志用于定位来源端拼接问题
            qWarning() << "[parseRetmsgPassFromJsonCompat] JSON parse failed:"
                       << jerr.errorString();
        }
    }

    // ---------- 2) 容错路径：从原文本中直接截取 XML ----------
    QString xmlExtracted;
    if (!extractXmlByTagRange(jsonString, &xmlExtracted)) {
        // 连 XML 片段都找不到，直接失败
        if (outRetmsg) *outRetmsg = QStringLiteral("JSON_INVALID_AND_XML_NOT_FOUND");
        return false;
    }

    // 如果截取出来的 XML 中仍然包含 \" 这类转义，做一次反转义（兼容你第二种格式的文本块）
    const QString xml = unescapeCommonJsonString(xmlExtracted);

    QString retmsg;
    if (!parseRetmsgFromXml(xml, &retmsg)) {
        if (outRetmsg) *outRetmsg = QStringLiteral("XML_PARSE_FAILED");
        return false;
    }

    if (outRetmsg) *outRetmsg = retmsg;
    return (retmsg == QLatin1String("PASS"));
}

// ===================================================================================
//
