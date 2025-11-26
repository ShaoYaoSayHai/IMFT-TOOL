#ifndef TESTTASK_H
#define TESTTASK_H

#include <QWidget>

#include <QTimer>

class TestTask : public QWidget
{
    Q_OBJECT
public:
    explicit TestTask(QWidget *parent = nullptr);

    ~TestTask();


private:

    // 私有变量
    volatile bool m_stopRequested;
    QTimer m_delayTimer;

public slots:

    // 触发槽函数
    // 开始执行任务的槽
    void startWork();
    // 停止工作的槽
    void stopWork();

signals:

};

#endif // TESTTASK_H
