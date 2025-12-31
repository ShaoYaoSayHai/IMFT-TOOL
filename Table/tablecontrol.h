// SPDX-License-Identifier: LGPL-3.0-only
// 许可声明：本文件依据 GNU LGPL v3 许可条款分发与使用。
// 完整许可文本见项目根目录：LPGL3.md

#ifndef TABLECONTROL_H
#define TABLECONTROL_H

#include <QObject>

#include <QTableWidget>
#include <QTableWidgetItem>
#include "config/version_config.h"

class TableControl : public QObject {
  Q_OBJECT
public:
  explicit TableControl(QTableWidget *pxTable, QObject *parent = nullptr);

  ~TableControl();

  enum TABLE_COLOR {
    GREEN,
    RED,
    BLUE,
    YELLOW ,
    WHITE
  };
  // 注册成QT独有的Enum
  Q_ENUM(TABLE_COLOR);

private:
  QTableWidget *table = nullptr;
  QTableWidgetItem *item = nullptr;

public slots:

//  void SetCellItem(int row, int col, QByteArray qbyData ,TABLE_COLOR color );

  void ClearAllItems();
  void SetCellItem(int row, int col, const QByteArray &qbyData, TABLE_COLOR color);

    QTableWidget *GetTableWidget();

signals:
};

//int findFirstColumnMatchRow(QTableWidget* tableWidget, const QString& compareString) ;

int findFirstColumnByLast2(QTableWidget* tableWidget, QString compareStringLast2) ;

#endif // TABLECONTROL_H
