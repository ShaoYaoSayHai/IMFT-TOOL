#ifndef MES_PARSE_H
#define MES_PARSE_H

#include <QString>

/**
 * @brief 解析 payload 中的 RETMSG，并判断是否 PASS
 * @param jsonString 输入：可能是合法 JSON（含转义），也可能是你那种不合法 JSON（未转义）
 * @param outRetmsg 输出：返回解析到的 RETMSG（如 PASS/FAIL/...）。失败时尽量给出可用信息
 * @return RETMSG == "PASS" 返回 true，否则返回 false（含解析失败）
 */
bool parseRetmsgPassFromJsonCompat(const QString& jsonString, QString* outRetmsg);

#endif // MES_PARSE_H
