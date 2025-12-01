// SPDX-License-Identifier: LGPL-3.0-only
// 许可声明：本文件依据 GNU LGPL v3 许可条款分发与使用。
// 完整许可文本见项目根目录：LPGL3.md

#ifndef TESTTASK_H
#define TESTTASK_H

#include <QWidget>

#include <QTimer>

class TestTask : public QWidget {
  Q_OBJECT
public:
  explicit TestTask(QWidget *parent = nullptr);

  ~TestTask();

private:

    QTimer *pxTimerSendOpenFireCmd = nullptr ;// 点火开阀的控制定时器

public slots:

    void TestTaskInit();


signals:
};

#endif // TESTTASK_H
