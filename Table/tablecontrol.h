#ifndef TABLECONTROL_H
#define TABLECONTROL_H

#include <QObject>

#include <QTableWidget>
#include <QTableWidgetItem>

class TableControl : public QObject
{
    Q_OBJECT
public:
    explicit TableControl( QTableWidget *pxTable , QObject *parent = nullptr);

    ~TableControl();

    enum TABLE_COLOR{
        GREEN ,
        RED ,
        BLUE ,
    } ;
    // 注册成QT独有的Enum
    Q_ENUM(TABLE_COLOR) ;

    void SetCellColor( int row , int col , TABLE_COLOR Color );



private:

    QTableWidget *table = nullptr ;
    QTableWidgetItem *item = nullptr ;

public slots:

    void SetCellItem( int row , int col , QByteArray qbyData );

signals:

};

#endif // TABLECONTROL_H
