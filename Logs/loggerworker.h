#ifndef LOGGERWORKER_H
#define LOGGERWORKER_H

#include <QObject>
#include <QString>

/*
 * LoggerWorker：运行在专用子线程中的 Worker
 * 文件 IO（打开/写入/flush/读取）均在 Worker 所在线程执行
 */
class LoggerWorker : public QObject
{
    Q_OBJECT
public:
    explicit LoggerWorker(QObject *parent = nullptr);
    ~LoggerWorker() override;

public slots:
    // 子线程初始化：设置文件路径并打开（必要时创建目录）
    void init(const QString &filePath);

    // 子线程追加写入：传入字符串
    void appendLog(const QString &text);

    // 子线程读取：读取整个文件内容
    void readAll();

    // 子线程安全关闭：结束前调用
    void shutdown();

    // 子线程追加写入：自动追加时间戳
    void appendLogWithTime(const QString &text);

signals:
    void readAllFinished(const QString &content, bool ok, const QString &err);
    void shutdownFinished();

private:
    bool openFileIfNeeded();
    void closeFile();

private:
    QString m_filePath;

    // QFile/QTextStream 放在 cpp 中包含具体头文件即可，这里前置声明不行（需要完整类型）
    // 因此在头文件里不声明 QFile/QTextStream，改为指针以减少头文件依赖也可。
    // 为了简单与兼容，这里采用指针形式：
    class QFile *m_file;
    class QTextStream *m_stream;
};

#endif // LOGGERWORKER_H
