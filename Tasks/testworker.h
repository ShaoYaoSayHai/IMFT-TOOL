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
    // 基础功能测试动作
    void onTaskBaseCommondTest();
    // 模拟点火开阀顺序动作
    void onSimulateIgnitionAction();

signals:

};

#endif // TESTWORKER_H
