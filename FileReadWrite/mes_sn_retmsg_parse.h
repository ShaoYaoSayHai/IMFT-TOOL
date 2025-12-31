#ifndef MES_SN_RETMSG_PARSE_H
#define MES_SN_RETMSG_PARSE_H

#include <QString>

struct MesParseResult
{
    QString sn;       // 例如 CE02_251227_10012
    QString retmsg;   // 例如 PASS / FAIL
    bool ok = false;  // retmsg == PASS
    QString rawError; // 解析失败原因（可选）
};

MesParseResult parseSnAndRetmsg_CE02Compat(const QString& input);

#endif // MES_SN_RETMSG_PARSE_H
