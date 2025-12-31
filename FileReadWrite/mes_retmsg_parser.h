#ifndef MES_RETMSG_PARSER_H
#define MES_RETMSG_PARSER_H

#include <QString>

bool parseMesRetmsgAndExtractSn(const QString& jsonString, QString* outSn, QString* outRetmsg);

#endif // MES_RETMSG_PARSER_H
