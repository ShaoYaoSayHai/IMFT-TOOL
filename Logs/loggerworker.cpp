#include "loggerworker.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>

LoggerWorker::LoggerWorker(QObject *parent)
    : QObject(parent),
      m_file(nullptr),
      m_stream(nullptr)
{
}

LoggerWorker::~LoggerWorker()
{
    closeFile();

    delete m_stream;
    m_stream = nullptr;

    delete m_file;
    m_file = nullptr;
}

void LoggerWorker::init(const QString &filePath)
{
    m_filePath = filePath;
    openFileIfNeeded();
}

void LoggerWorker::appendLog(const QString &text)
{
    if (!openFileIfNeeded())
        return;

    // 建议统一带时间戳，便于追溯与对账
    const QString line =
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
        + " | " + text + "\n";

    (*m_stream) << line;

    // 追溯场景优先保证落盘（牺牲部分性能）
    m_stream->flush();
    m_file->flush();
}

void LoggerWorker::readAll()
{
    // 确保读取到最新内容
    if (m_file && m_file->isOpen()) {
        m_stream->flush();
        m_file->flush();
    }

    QFile f(m_filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit readAllFinished(QString(), false, QString("open failed: %1").arg(f.errorString()));
        return;
    }

    const QByteArray bytes = f.readAll();
    f.close();

    // 统一按 UTF-8 解释
    emit readAllFinished(QString::fromUtf8(bytes), true, QString());
}

void LoggerWorker::shutdown()
{
    closeFile();
    emit shutdownFinished();
}

bool LoggerWorker::openFileIfNeeded()
{
    if (m_filePath.isEmpty())
        return false;

    if (!m_file)
        m_file = new QFile();

    if (m_file->isOpen())
        return true;

    // 确保目录存在
    QFileInfo fi(m_filePath);
    QDir dir(fi.absolutePath());
    if (!dir.exists())
        dir.mkpath(".");

    m_file->setFileName(m_filePath);

    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return false;
    }

    if (!m_stream)
        m_stream = new QTextStream();

    m_stream->setDevice(m_file);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5 下指定 UTF-8，避免多语言/跨系统乱码
    m_stream->setCodec("UTF-8");
#endif

    return true;
}

void LoggerWorker::closeFile()
{
    if (m_file && m_file->isOpen()) {
        if (m_stream) {
            m_stream->flush();
        }
        m_file->flush();
        m_file->close();
    }
}

void LoggerWorker::appendLogWithTime(const QString &text)
{
    if (!openFileIfNeeded())
        return;

    const QString line =
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
        + " | " + text + "\n";

    (*m_stream) << line;
    m_stream->flush();
    m_file->flush();
}

