#include "testworker.h"
#include <QDebug>

TestWorker::TestWorker(QObject *parent) : QObject(parent) {
  // 线程启动/结束的槽
  connect(&testWorkerThread, &QThread::started, this,
          &TestWorker::onThreadStarted);
  connect(&testWorkerThread, &QThread::finished, this,
          &TestWorker::onThreadFinished);
}

TestWorker::~TestWorker() {
  qDebug() << "================= TestWorker finish begin ================";
  qDebug() << "TestWorker delete start";
  this->stopWorker();
  qDebug() << "TestWorker delete success";
  qDebug() << "================= TestWorker finish end ================";
}

void TestWorker::startWorker() {
  if (testWorkerThread.isRunning()) {
    return;
  }

  pxTestLoop = new TestLoop();
  pxTestLoop->moveToThread(&testWorkerThread); // 线程移动

  testWorkerThread.start(); // 线程启动

  qDebug() << "================= TestWorker start success ================";
  // TestTaskInit
  // 1. 通知 TestLoop 开启定时器
  if (pxTestLoop) {
    QMetaObject::invokeMethod(pxTestLoop, "TestTaskInit", Qt::QueuedConnection);
  }
}

void TestWorker::stopWorker() {
  if (!testWorkerThread.isRunning())
    return;
  //    bool status = (pxTestLoop)?1:0 ;
  // 1. 通知 TestLoop 停止定时器
  if (pxTestLoop) {
    QMetaObject::invokeMethod(pxTestLoop, "TestTaskDeinit",
                              Qt::QueuedConnection);
  }
  // 2. 请求线程结束
  testWorkerThread.quit();
  // 3. 等待线程结束（阻塞当前线程，通常是 UI 线程，时间应该很短）
  testWorkerThread.wait();
  // 线程结束后 onThreadFinished 会被调用，m_serial 会被 deleteLater
  pxTestLoop = nullptr;
}

void TestWorker::onThreadStarted() {}

void TestWorker::onThreadFinished() {
  qDebug() << "testWorker finish begin ===================";
  pxTestLoop->deleteLater();
  qDebug() << "testWorker finish begin ===================";
}

void TestWorker::onTakeStep1Test(QList<DeviceInfo> deviceList) {
  // 为空返回
  if (deviceList.isEmpty()) {
    return;
  }
  //    if (pxTestLoop) {
  //        QMetaObject::invokeMethod(pxTestLoop, "onControlAllValveTool",
  //                                  Qt::QueuedConnection);
  //    }
  //   if (pxTestLoop) {
  //     // 使用 invokeMethod 进行跨线程调用
  //     QMetaObject::invokeMethod(pxTestLoop, "onReadAllGTDevicePressure",
  //                               Qt::QueuedConnection, //
  //                               使用队列连接确保线程安全
  //                               Q_ARG(QList<DeviceInfo>, deviceList));
  //   }

  if (pxTestLoop) {
    QMetaObject::invokeMethod(pxTestLoop, "GT_ReadListAllPressure",
                              Qt::QueuedConnection, // 使用队列连接确保线程安全
                              Q_ARG(QList<DeviceInfo>, deviceList));
  }
}

void TestWorker::onTaskBaseCommondTest() {
  if (pxTestLoop) {
    QMetaObject::invokeMethod(pxTestLoop, "onTestBaseCmdAll",
                              Qt::QueuedConnection);
  }
}

/**
 * @brief 模拟点火操作 按照顺序去开关
 */
void TestWorker::onSimulateIgnitionAction() {
  if (pxTestLoop) {
    QMetaObject::invokeMethod(
        pxTestLoop, "CTL_SetDeviceSwitchCloseAll", Qt::QueuedConnection,
        Q_ARG(QList<CLTDeviceInfo>, pxTestLoop->DeviceCLInfo));
  }
}

/**
 * @brief 读取所有阀门状态
 * @param deviceList
 */
void TestWorker::onReadAllValveStatus( QList<DeviceInfo> deviceList ) {
  if (pxTestLoop) {
    QMetaObject::invokeMethod(pxTestLoop, "GT_ReadDeviceSwitchStatusAll",
                              Qt::QueuedConnection,
                              Q_ARG(QList<DeviceInfo>, deviceList));
  }
}

void TestWorker::onReadAllBoardPressure(QList<DeviceInfo> deviceList) {
  // 为空返回
  if (deviceList.isEmpty()) {
    return;
  }
  if (pxTestLoop) {
    QMetaObject::invokeMethod(pxTestLoop, "GT_ReadListAllPressure",
                              Qt::QueuedConnection, // 使用队列连接确保线程安全
                              Q_ARG(QList<DeviceInfo>, deviceList));
  }
}


