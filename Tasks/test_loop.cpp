#include "testtask.h"

TestTask::TestTask(QWidget *parent) : QWidget(parent)
{
    // 连接定时器超时信号到第二个任务

}

TestTask::~TestTask()
{
    if( pxTimerSendOpenFireCmd )
    {
        if( pxTimerSendOpenFireCmd->isActive() )
        { pxTimerSendOpenFireCmd->stop() ; }
        pxTimerSendOpenFireCmd->deleteLater() ; // 销毁
    }
}

void TestTask::TestTaskInit()
{
    pxTimerSendOpenFireCmd = new QTimer(  );
}

