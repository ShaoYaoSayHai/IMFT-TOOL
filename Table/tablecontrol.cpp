#include "tablecontrol.h"
#include <QDebug>

TableControl::TableControl( QTableWidget *pxTable , QObject *parent) : QObject(parent)
  , table( pxTable )
{

}

TableControl::~TableControl()
{
}

void TableControl::SetCellItem(int row, int col, const QByteArray& qbyData, TABLE_COLOR color)
{
    const int r = row - 1;
    const int c = col - 1;

    // 1) 越界保护，避免潜在崩溃
    if (r < 0 || c < 0 || r >= table->rowCount() || c >= table->columnCount()) {
        qWarning() << "SetCellItem out of range:" << row << col;
        return;
    }

    // 2) 复用已有 item
    QTableWidgetItem* item = table->item(r, c);
    if (!item) {
        item = new QTableWidgetItem();
        table->setItem(r, c, item); // table 接管生命周期
    }

    // 3) 设置文本（注意编码）
    // 若 qbyData 是 UTF-8 文本：
    item->setText(QString::fromUtf8(qbyData));
    // 若是本地编码（GBK等），需要用 QTextCodec 或 fromLocal8Bit：
    // item->setText(QString::fromLocal8Bit(qbyData));

    // 4) 设置背景色
    switch (color) {
    case GREEN: item->setBackground(QColor(0, 255, 0)); break;
    case RED:   item->setBackground(QColor(255, 0, 0)); break;
    case YELLOW: item->setBackground(QColor( 255,128,0 ));break;
    default:    item->setBackground(QColor(255, 255, 255)); break;
    }
}

QTableWidget *TableControl::GetTableWidget()
{
    return table ;
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


#if ( VERSION < VERSION_BLD(1,4) )
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
        if ((item->text().mid( (item->text().size()-2) , (item->text().size()))).contains(compareString)) {
            return row; // 返回匹配的行号
        }
    }
    qDebug() << "未找到与'" << compareString << "'匹配的内容";
    return -1; // 没有找到匹配项
}

#else
int findFirstColumnByLast2(QTableWidget* tableWidget, QString compareStringLast2)
{
    if (!tableWidget) {
        qWarning() << "findFirstColumnByLast2: tableWidget is null";
        return -1;
    }

    const int rowCount = tableWidget->rowCount();
    if (rowCount <= 0) {
        return -1;
    }

    // 1) 规范化输入：去空格
    compareStringLast2 = compareStringLast2.trimmed();
    if (compareStringLast2.isEmpty()) {
        return -1;
    }

    // 2) 如果你确定输入永远是两位，这段可保留为强校验：
    //    若输入可能是 "3" 而你希望当作 "03"，则做左补零。
    if (compareStringLast2.size() == 1 && compareStringLast2[0].isDigit()) {
        compareStringLast2.prepend('0');  // "3" -> "03"
    }

    // 若输入仍不是两位，按你的业务选择：直接失败或截取最后两位
    if (compareStringLast2.size() != 2) {
        qWarning() << "findFirstColumnByLast2: compareStringLast2 length is not 2:" << compareStringLast2;
        return -1;
        // 或者：compareStringLast2 = compareStringLast2.right(2);
    }

    // 3) 遍历第一列，取完整 SN 的末尾两位
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem* item = tableWidget->item(row, 0);
        if (!item) {
            continue;
        }

        const QString sn = item->text().trimmed();
        if (sn.size() < 2) {
            continue;
        }

        const QString last2 = sn.right(2); // 安全且语义明确
        if (last2 == compareStringLast2) {
            return row;
        }
    }

    return -1;
}
#endif
