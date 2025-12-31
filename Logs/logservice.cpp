#include "logservice.h"
#include "loggerworker.h"

#include <QThread>
#include <QMetaObject>
#include <QStandardPaths>

LogService::LogService(QObject *parent)
    : QObject(parent),
      m_started(false),
      m_thread(nullptr),
      m_worker(nullptr)
{
}

LogService::~LogService()
{
    stop();
}

QString LogService::defaultLogPath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return base + "/logs/production_log.txt";
}

void LogService::start(const QString &filePath)
{
    if (m_started)
        return;

    const QString finalPath = filePath.isEmpty() ? defaultLogPath() : filePath;

    m_thread = new QThread();
    m_worker = new LoggerWorker();

    m_worker->moveToThread(m_thread);

    // 线程启动后初始化文件
    QObject::connect(m_thread, &QThread::started, this, [this, finalPath]() {
        if (m_worker) {
            QMetaObject::invokeMethod(m_worker, "init",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, finalPath));
        }
    });

    // 读取结果透传
    QObject::connect(m_worker, &LoggerWorker::readAllFinished,
                     this, &LogService::readAllFinished);

    m_started = true;
    m_thread->start();
}

void LogService::stop()
{
    if (!m_started)
        return;

    // 如果线程存在且在跑：先“阻塞式”让 worker 在子线程完成关闭文件
    if (m_thread && m_worker && m_thread->isRunning()) {
        // 关键：BlockingQueuedConnection，保证 shutdown 在 worker 线程执行完才返回
        QMetaObject::invokeMethod(m_worker, "shutdown", Qt::BlockingQueuedConnection);

        // 然后由 stop() 主动 quit/wait，不再依赖任何信号回调
        m_thread->quit();
        m_thread->wait();
    }

    // 析构路径：不要依赖 deleteLater（可能没有事件循环）
    cleanupObjects();

    m_started = false;
}

void LogService::cleanupObjects()
{
    // 注意：worker 已经不在运行的线程上工作了（线程已 wait 结束），可以直接 delete
    if (m_worker) {
        delete m_worker;
        m_worker = nullptr;
    }
    if (m_thread) {
        delete m_thread;
        m_thread = nullptr;
    }
}

void LogService::append(const QString &text)
{
    // 应对退出阶段：线程/worker 可能已不存在
    if (!m_started || !m_thread || !m_worker || !m_thread->isRunning())
        return;

    QMetaObject::invokeMethod(m_worker, "appendLog",
                              Qt::QueuedConnection,
                              Q_ARG(QString, text));
}

void LogService::requestReadAll()
{
    if (!m_started || !m_thread || !m_worker || !m_thread->isRunning())
        return;

    QMetaObject::invokeMethod(m_worker, "readAll", Qt::QueuedConnection);
}


void LogService::appendWithTime(const QString &text)
{
    if (!m_started || !m_thread || !m_worker || !m_thread->isRunning())
        return;

    QMetaObject::invokeMethod(m_worker, "appendLogWithTime",
                              Qt::QueuedConnection,
                              Q_ARG(QString, text));
}

