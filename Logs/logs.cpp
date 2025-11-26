#include "logs.h"
#include "QDateTime"

Logs::Logs(QTextBrowser *browser ,QObject *parent) : QObject(parent)
  , pxLogsBrowser(browser)
{

}

Logs::~Logs()
{

}

void Logs::LogBrowserWrite(QByteArray info)
{
    // 获取当前时间并格式化
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timestamp = currentTime.toString("yyyy-MM-dd hh:mm:ss");
    // 组合时间戳和消息
    QString logEntry = QString("[%1] %2").arg(timestamp, info);
    pxLogsBrowser->append(logEntry) ;
}

void Logs::LogBrowserClear()
{
    pxLogsBrowser->clear() ;
}
