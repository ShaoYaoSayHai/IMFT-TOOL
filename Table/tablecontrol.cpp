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

int findFirstColumnMatchRow(QTableWidget* tableWidget, const QString& compareString) {
    // 检查表格控件是否有效
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
        // 比较单元格文本与目标字符串
        if ((item->text().mid( (item->text().size()-2) , (item->text().size()))) == compareString) {
//            qDebug() << "找到匹配项，行号：" << row << "，内容：" << item->text();
            return row; // 返回匹配的行号
        }
    }
    qDebug() << "未找到与'" << compareString << "'匹配的内容";
    return -1; // 没有找到匹配项
}
