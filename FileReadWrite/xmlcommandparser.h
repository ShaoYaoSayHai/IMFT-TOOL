#ifndef XMLCOMMANDPARSER_H
#define XMLCOMMANDPARSER_H

#include <QObject>
#include "./FileReadWrite/filerw.h"

class XmlCommandParser : public QObject
{
    Q_OBJECT
public:
    explicit XmlCommandParser(QObject *parent = nullptr);

    ~XmlCommandParser();


public slots:



signals:

};

#endif // XMLCOMMANDPARSER_H
