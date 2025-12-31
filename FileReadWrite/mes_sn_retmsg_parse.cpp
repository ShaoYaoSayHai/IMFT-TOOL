#include "mes_sn_retmsg_parse.h"

#include <QXmlStreamReader>
#include <QRegularExpression>

static QString normalizePayload(QString s)
{
    // 1) 去掉首尾空白
    s = s.trimmed();

    // 2) 处理常见转义：把 JSON/日志里的 \" 还原为 "
    //    注意顺序：先处理 \\\" -> \" 再处理 \" -> "
    s.replace(QStringLiteral("\\\\\""), QStringLiteral("\\\""));
    s.replace(QStringLiteral("\\\""), QStringLiteral("\""));

    // 3) 可选：处理 &quot;
    s.replace(QStringLiteral("&quot;"), QStringLiteral("\""));

    return s;
}

static void tryParseRetmsgFromXml(const QString& s, QString* outRetmsg)
{
    if (!outRetmsg) return;
    if (!outRetmsg->isEmpty()) return;

    // 从 <root 或 <info 处开始截取一段喂给 XML reader（避免前缀日志干扰）
    int pos = s.indexOf(QStringLiteral("<root"));
    if (pos < 0) pos = s.indexOf(QStringLiteral("<info"));
    if (pos < 0) return;

    const QString xml = s.mid(pos);

    QXmlStreamReader xr(xml);
    while (!xr.atEnd()) {
        xr.readNext();

        if (xr.isStartElement()
            && xr.name().toString().compare(QStringLiteral("info"), Qt::CaseInsensitive) == 0) {

            // 遍历属性，大小写不敏感匹配 RETMSG
            const auto attrs = xr.attributes();
            for (const auto& a : attrs) {
                const QString key = a.name().toString();
                if (key.compare(QStringLiteral("RETMSG"), Qt::CaseInsensitive) == 0) {
                    *outRetmsg = a.value().toString().trimmed();
                    return;
                }
            }
            return;
        }
    }
}

MesParseResult parseSnAndRetmsg_CE02Compat(const QString& input)
{
    MesParseResult r;

    // 0) 清洗/反转义（这是你之前“XML能看到但解析取不到”的常见根因）
    const QString s = normalizePayload(input);

    // 1) 强特征抓 SN：只要出现 CE02 开头就抓
    //    你如果 SN 格式固定（CE02_YYMMDD_XXXXX），可以把正则收紧：
    //    CE02_\d{6}_\d{5}
    {
        QRegularExpression reSn(QStringLiteral("(CE02_[0-9]{6}_[0-9]{5})"));
        QRegularExpressionMatch m = reSn.match(s);
        if (m.hasMatch()) {
            r.sn = m.captured(1);
        } else {
            // 放宽：CE02 后面跟字母数字下划线
            QRegularExpression reSnLoose(QStringLiteral("(CE02[0-9A-Za-z_]+)"));
            m = reSnLoose.match(s);
            if (m.hasMatch())
                r.sn = m.captured(1);
        }
    }

    // 2) RETMSG：优先 XML 解析（更准确）
    tryParseRetmsgFromXml(s, &r.retmsg);

    // 3) RETMSG 正则兜底（兼容 XML/JSON/日志各种形态）
    if (r.retmsg.isEmpty()) {
        // 形态 A：RETMSG="PASS"
        QRegularExpression reA(QStringLiteral("RETMSG\\s*=\\s*\"([^\"]+)\""), QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m = reA.match(s);
        if (m.hasMatch()) {
            r.retmsg = m.captured(1).trimmed();
        }
    }
    if (r.retmsg.isEmpty()) {
        // 形态 B：RETMSG\":\"PASS\"（JSON 里常见）
        QRegularExpression reB(QStringLiteral("RETMSG\\\\?\"\\s*:\\s*\\\\?\"([^\\\\\"]+)"), QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch m = reB.match(s);
        if (m.hasMatch()) {
            r.retmsg = m.captured(1).trimmed();
        }
    }

    // 4) 输出判定
    r.ok = (r.retmsg.compare(QStringLiteral("PASS"), Qt::CaseInsensitive) == 0);

    // 5) 错误信息
    if (r.sn.isEmpty() || r.retmsg.isEmpty()) {
        r.rawError = QStringLiteral("Parse incomplete: ");
        if (r.sn.isEmpty()) r.rawError += QStringLiteral("[SN missing] ");
        if (r.retmsg.isEmpty()) r.rawError += QStringLiteral("[RETMSG missing] ");
    }

    return r;
}
