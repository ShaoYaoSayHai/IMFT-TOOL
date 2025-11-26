#include "testtask.h"

TestTask::TestTask(QWidget *parent) : QWidget(parent)
{
    m_delayTimer.setSingleShot(true);
    // 连接定时器超时信号到第二个任务
//    connect(&m_delayTimer, &QTimer::timeout, this, &TestTask::onSecondTask);
}

TestTask::~TestTask()
{

}

void TestTask::startWork()
{

}

void TestTask::stopWork()
{

}
