#include "tablecontrol.h"
#include <QDebug>

TableControl::TableControl( QTableWidget *pxTable , QObject *parent) : QObject(parent)
    , table( pxTable )
{

}

TableControl::~TableControl()
{
    qDebug()<<"TableControl delete success" ;
}

/**
 * @brief TableControl::SetCellColor
 * @param row 行
 * @param col 列
 * @param Color 颜色
 */
void TableControl::SetCellColor(int row, int col, TABLE_COLOR Color)
{
    item = table->item(( row -1 ) , col-1) ;
    if( Color == GREEN )
    {
        item->setBackground(QColor(0,255,0)); // 设置绿色
    }
    else if(Color == RED)
    {
        item->setBackground(QColor(255,0,0)); // 设置红色
    }
    else
    {
        item->setBackground(QColor(255,255,255)); // 设置红色
    }
}

void TableControl::SetCellItem(int row, int col, QByteArray qbyData)
{
    item = table->item(( row -1 ) , col-1) ;
    qDebug()<<(item ? 1 : 0) <<qbyData ;
    item->setText(qbyData) ;
}
