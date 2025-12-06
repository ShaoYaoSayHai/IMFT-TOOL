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
    // 读取所有阀门状态
//    void onReadAllValveStatus();
    void onReadAllValveStatus(QList<DeviceInfo> deviceList);
    // 读取进气端相对压力
    void onReadAllBoardPressure(QList<DeviceInfo> deviceList);
    // 欠压执行
    void OMFT_LowPressureFunc( QList<DeviceInfo> deviceList );
    // 点火开阀执行
    void OMFT_OpenFireFunc( QList<DeviceInfo> deviceList );
    // 超压测试
    void OMFT_OverPressureFunc( QList<DeviceInfo> deviceList );
    // 提交到服务器
    void OMFT_SubmitTestResultToMES( QList<DeviceInfo> deviceList );
signals:

};

#endif // TESTWORKER_H
