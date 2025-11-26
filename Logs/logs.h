#ifndef LOGS_H
#define LOGS_H

#include <QObject>
#include <QTextBrowser>

class Logs : public QObject
{
    Q_OBJECT
public:
    explicit Logs( QTextBrowser *browser , QObject *parent = nullptr);

    ~Logs();

    void LogBrowserWrite( QByteArray info );

    void LogBrowserClear();


private:

    // MainWindow页面的Browser指针
    QTextBrowser *pxLogsBrowser = nullptr ;



signals:

};

#endif // LOGS_H
