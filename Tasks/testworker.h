#ifndef TESTWORKER_H
#define TESTWORKER_H

#include <QObject>

class TestWorker : public QObject
{
    Q_OBJECT
public:
    explicit TestWorker(QObject *parent = nullptr);

signals:

};

#endif // TESTWORKER_H
