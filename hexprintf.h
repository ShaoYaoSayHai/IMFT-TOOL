// SPDX-License-Identifier: LGPL-3.0-only
// 许可声明：本文件依据 GNU LGPL v3 许可条款分发与使用。
// 完整许可文本见项目根目录：LPGL3.md

#ifndef HEXPRINTF_H
#define HEXPRINTF_H

#include <QByteArray>
#include <QDebug>

/**
 * @brief 以十六进制打印数据
 * @param buffer
 */
void HexPrintf(QByteArray &buffer);

#endif // HEXPRINTF_H
