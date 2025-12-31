#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include <QObject>
#include <QString>

class QThread;
class LoggerWorker;

class LogService : public QObject
{
    Q_OBJECT
public:
    explicit LogService(QObject *parent = nullptr);
    ~LogService() override;

    void start(const QString &filePath = QString());
    void stop();                 // 关键：stop 内部“阻塞式”关闭 worker，再 quit/wait

    Q_INVOKABLE void append(const QString &text);
    Q_INVOKABLE void requestReadAll();

    // 追加一条日志：自动追加时间戳
    Q_INVOKABLE void appendWithTime(const QString &text);


signals:
    void readAllFinished(const QString &content, bool ok, const QString &err);

private:
    QString defaultLogPath() const;
    void cleanupObjects();        // 关键：析构路径不依赖 deleteLater

private:
    bool m_started;
    QThread *m_thread;
    LoggerWorker *m_worker;
};

#endif // LOGSERVICE_H
