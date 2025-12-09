#include "tablecontrol.h"
#include <QDebug>

TableControl::TableControl( QTableWidget *pxTable , QObject *parent) : QObject(parent)
    , table( pxTable )
{

}

TableControl::~TableControl()
{
}

void TableControl::SetCellItem(int row, int col, QByteArray qbyData , TABLE_COLOR color )
{
//    item = table->item(( row -1 ) , col-1) ;
//    qDebug()<<"是否是TRUE : "<<(item ? 1 : 0) <<qbyData ;
//    item->setText(qbyData) ;
    QTableWidgetItem *item = table->takeItem( row - 1 , col - 1 );
    if( item )
    delete item ;
    qDebug()<<"进入到删除ITEM之后" ;
    QString writeMsg ;
    writeMsg.prepend( qbyData ) ;
    table->setItem( row - 1 , col - 1 , new QTableWidgetItem( writeMsg ) );
    qDebug()<<"写入ITEM" ;
    QTableWidgetItem *newItem = table->item( row -1 , col - 1 ) ;
    qDebug()<<"获取到newItem" ;
    if( color == GREEN )
    {
        newItem->setBackground(QColor(0,255,0)); // 设置绿色
    }
    else if(color == RED)
    {
        newItem->setBackground(QColor(255,0,0)); // 设置红色
    }
    else
    {
        newItem->setBackground(QColor(255,255,255)); // 设置红色
    }
    qDebug()<<"NEW ITEM 背景色写入完成" ;
}

void TableControl::ClearAllItems()
{
#if 0
    int row = table->rowCount() ;
    int col = table->columnCount() ;
    qDebug()<<"row : " << row ;
    qDebug()<<"col : " << col ;
    for( int currentRow=0;currentRow<row;currentRow++ )
    {
        for( int currentCol=0;currentCol<col;currentCol++ )
        {
            item = table->item(( currentRow ) , currentCol ) ;
            item->setText(" ") ;
            item->setBackground( QColor(255,255,255) );
        }
    }

#else
    table->clearContents() ;
#endif
}


int findFirstColumnMatchRow(QTableWidget* tableWidget, const QString& compareString) {
    // 检查表格控件是否有效
    int searchLine = compareString.toUInt() ;
    if (!tableWidget) {
        qDebug() << "错误：表格控件指针为空";
        return -1;
    }
    int rowCount = tableWidget->rowCount();
    if (rowCount == 0) {
        qDebug() << "表格为空，无数据可比较";
        return -1;
    }
    // 遍历所有行，查找第一列匹配的项
    for (int row = 0; row < rowCount; ++row) {
        // 获取第一列（列索引为0）的单元格项
        QTableWidgetItem* item = tableWidget->item(row, 0);
        // 检查单元格是否存在
        if (item == nullptr) {
            qDebug() << "警告：第" << row << "行第0列的单元格为空，跳过检查";
            continue;
        }
        qDebug()<<"查找内容 - "<<(item->text().mid( (item->text().size()-2) , (item->text().size()))) ;
        // 比较单元格文本与目标字符串
        QString currentLine = (item->text().mid( (item->text().size()-2) , (item->text().size()))) ;
        int lineValue = currentLine.toUInt() ;
        qDebug()<<"转换后的SLAVEID - "<<lineValue <<"待查找的ID"<<searchLine ;
        if( searchLine == lineValue )
        {
            return row; // 返回匹配的行号
        }
        if ((item->text().mid( (item->text().size()-3) , (item->text().size()))).contains(compareString)) {
//            qDebug() << "找到匹配项，行号：" << row << "，内容：" << item->text();
            return row; // 返回匹配的行号
        }
    }
    qDebug() << "未找到与'" << compareString << "'匹配的内容";
    return -1; // 没有找到匹配项
}
