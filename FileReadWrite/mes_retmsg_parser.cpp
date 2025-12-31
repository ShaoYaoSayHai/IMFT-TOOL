#include "mes_retmsg_parser.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QXmlStreamReader>
#include <QRegularExpression>

static bool extractInfoAttrsFromXml(const QString& xml, QString* outRetval, QString* outRetmsg)
{
    if (outRetval) outRetval->clear();
    if (outRetmsg) outRetmsg->clear();

    // 允许 xml 前面有前缀噪声：从 <root 或 <info> 开始截取
    int pos = xml.indexOf(QStringLiteral("<root"));
    if (pos < 0) pos = xml.indexOf(QStringLiteral("<info"));
    const QString xmlPart = (pos >= 0) ? xml.mid(pos) : xml;

    QXmlStreamReader xr(xmlPart);
    while (!xr.atEnd()) {
        xr.readNext();
        if (xr.isStartElement() &&
            xr.name().toString().compare(QStringLiteral("info"), Qt::CaseInsensitive) == 0) {

            const auto attrs = xr.attributes();
            if (outRetval) *outRetval = attrs.value(QStringLiteral("RETVAL")).toString().trimmed();
            if (outRetmsg) *outRetmsg = attrs.value(QStringLiteral("RETMSG")).toString().trimmed();
            return true;
        }
    }
    return false;
}

bool parseMesRetmsgAndExtractSn(const QString& jsonString, QString* outSn, QString* outRetmsg)
{
    if (outSn) outSn->clear();
    if (outRetmsg) outRetmsg->clear();

    // 1) 解析 JSON，取 d 字段（Qt 会自动处理 \" 反转义）
    QJsonParseError jerr;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &jerr);
    if (jerr.error != QJsonParseError::NoError || !doc.isObject()) {
        // JSON 不合法，直接判失败（也可在此尝试额外容错）
        if (outRetmsg) *outRetmsg = QStringLiteral("JSON_PARSE_ERROR");
        return false;
    }

    const QJsonObject obj = doc.object();
    const QJsonValue dVal = obj.value(QStringLiteral("d"));
    if (!dVal.isString()) {
        if (outRetmsg) *outRetmsg = QStringLiteral("JSON_FIELD_D_MISSING");
        return false;
    }

    const QString xml = dVal.toString();

    // 2) 解析 XML，取 RETVAL / RETMSG
    QString retval, retmsg;
    if (!extractInfoAttrsFromXml(xml, &retval, &retmsg)) {
        if (outRetmsg) *outRetmsg = QStringLiteral("XML_PARSE_ERROR");
        return false;
    }

    if (outRetmsg) *outRetmsg = retmsg;

    // 3) 成功条件：RETMSG == PASS（你给的成功样例就是这种）
    //    成功时不携带 SN，因此 outSn 保持为空
    if (retmsg.compare(QStringLiteral("PASS"), Qt::CaseInsensitive) == 0) {
        return true;
    }

    // 4) 失败：从 RETMSG 中提取 SN:CE02_xxx（只对失败才提取）
    //    示例：SN:CE02_251227_10012所在工站...
    {
        QRegularExpression re(QStringLiteral("SN\\s*[:：]\\s*(CE02_[0-9]{6}_[0-9]{5})"),
                              QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m = re.match(retmsg);
        if (m.hasMatch() && outSn) {
            *outSn = m.captured(1).trimmed();
        }
    }

    return false;
}
