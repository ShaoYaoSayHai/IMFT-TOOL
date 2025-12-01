#ifndef TESTWORKER_H
#define TESTWORKER_H

#include <QObject>
#include "./test_loop.h"
#include <QThread>


class TestWorker : public QObject
{
    Q_OBJECT
public:
    explicit TestWorker(QObject *parent = nullptr);

    ~TestWorker();

    TestLoop *pxTestLoop = nullptr ;// 初始化为空指针
    QThread  testWorkerThread ;




public slots:

    void startWorker();

    void stopWorker();

    void onThreadStarted();

    void onThreadFinished();

    void onTakeStep1Test( QList<DeviceInfo> deviceList );

signals:

};

#endif // TESTWORKER_H
