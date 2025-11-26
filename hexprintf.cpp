#include "hexprintf.h"


/**
 * @brief 以十六进制打印数据
 * @param buffer
 */
void HexPrintf(QByteArray &buffer)
{
    QString line;
    for (int i = 0; i < buffer.size(); i++)
    {
        line += QString("%1 ").arg(static_cast<uint8_t>(buffer[i]), 2, 16, QLatin1Char('0')).toUpper();

        if ((i + 1) % 8 == 0)
        {
            qDebug() << QString("[%1-%2]: %3").arg(i - 7, 2, 10, QLatin1Char('0')).arg(i, 2, 10, QLatin1Char('0')).arg(line);
            line.clear();
        }
    }
    if (!line.isEmpty())
    {
        int startIndex = buffer.size() - (buffer.size() % 8);
        qDebug() << QString("[%1-%2]: %3").arg(startIndex, 2, 10, QLatin1Char('0')).arg(buffer.size() - 1, 2, 10, QLatin1Char('0')).arg(line);
    }
}
