#include "ModbusConfigParser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QFileInfo>

void TaskInfo::printInfo() const
{
    qDebug() << "Task ID:" << id;
    qDebug() << "  Enable:" << enable;

    QStringList slaveIdStrs;
    for (uint8_t id : slave_ids) {
        slaveIdStrs.append(QString("0x%1").arg(id, 2, 16, QChar('0')).toUpper());
    }
    qDebug() << "  Slave IDs:" << slaveIdStrs.join(", ");
    qDebug() << "  Function:" << QString("0x%1").arg(func, 2, 16, QChar('0')).toUpper();
    qDebug() << "  Address:" << QString("0x%1").arg(address, 4, 16, QChar('0')).toUpper();
    qDebug() << "  Read Length:" << read_length;

    qDebug() << "  Write Buffers (" << write_buffers.size() << " steps):";
    for (int i = 0; i < write_buffers.size(); i++) {
        qDebug() << "    Step" << (i+1) << ":" << write_buffers[i].toHex(' ').toUpper();
    }
}

QByteArray ModbusConfigParser::parseHexString(const QString& hexString)
{
    QByteArray result;
    QString cleanedString = hexString.trimmed();

    if (cleanedString.isEmpty()) {
        return result;
    }

    // 去除"0x"前缀
    cleanedString = cleanedString.replace("0x", "", Qt::CaseInsensitive);

    // 用正则表达式匹配十六进制数字（支持逗号、空格、斜杠分隔）
    QRegularExpression hexRegex(R"([0-9a-fA-F]{1,2})");
    QRegularExpressionMatchIterator matchIter = hexRegex.globalMatch(cleanedString);

    while (matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        QString hexByte = match.captured(0);
        bool ok;
        uint byteValue = hexByte.toUInt(&ok, 16);
        if (ok) {
            result.append(static_cast<char>(byteValue));
        }
    }

    return result;
}

QList<uint8_t> ModbusConfigParser::parseSlaveIds(const QString& addrString)
{
    QList<uint8_t> slaveIds;
    QString cleanedString = addrString.trimmed();

    if (cleanedString.isEmpty()) {
        return slaveIds;
    }

    // 用正则表达式匹配十六进制地址
    QRegularExpression addrRegex(R"((0x)?([0-9a-fA-F]{1,2}))");
    QRegularExpressionMatchIterator matchIter = addrRegex.globalMatch(cleanedString);

    while (matchIter.hasNext()) {
        QRegularExpressionMatch match = matchIter.next();
        QString hexAddr = match.captured(2); // 获取不带0x的部分
        bool ok;
        uint addrValue = hexAddr.toUInt(&ok, 16);
        if (ok && addrValue <= 0xFF) {
            slaveIds.append(static_cast<uint8_t>(addrValue));
        }
    }

    return slaveIds;
}

QList<TaskInfo> ModbusConfigParser::parseConfigImpl(const QDomDocument& doc, const QList<uint8_t>& gtSlaveIds)
{
    QList<TaskInfo> tasks;

    QDomElement root = doc.documentElement();
    if (root.isNull()) {
      qDebug() << "[错误] 未找到根元素（Root）。";
      return tasks;
    }
    QDomElement buildCommand = root.firstChildElement("CommandConfigList");
    if (buildCommand.isNull()) {
      qDebug() << "[错误] 未找到 CommandConfigList 节点。";
      return tasks ;
    }

    // 遍历所有CommandConfig节点
    QDomNodeList configNodes = buildCommand.elementsByTagName("CommandConfig");
    for (int i = 0; i < configNodes.count(); i++) {
        QDomElement configElem = configNodes.at(i).toElement();
        if (configElem.isNull()) continue;

        TaskInfo task;

        // 解析id
        QDomElement idElem = configElem.firstChildElement("id");
        if (!idElem.isNull()) {
            task.id = idElem.text().toInt();
        }

        // 解析enable
        QDomElement enableElem = configElem.firstChildElement("enable");
        if (!enableElem.isNull()) {
            QString enableText = enableElem.text().trimmed().toLower();
            task.enable = (enableText == "true" || enableText == "1");
        }

        // 解析slave_id（根据type属性处理）
        QDomElement slaveIdElem = configElem.firstChildElement("slave_id");
        if (!slaveIdElem.isNull()) {
            QString type = slaveIdElem.attribute("type");
            QString content = slaveIdElem.text().trimmed();

            if (type == "GT") {
                // 使用外部传入的gtSlaveIds
                task.slave_ids = gtSlaveIds;
            } else if (type == "LST") {
                // 解析LST类型的逗号分隔地址列表
                task.slave_ids = parseSlaveIds(content);
            } else {
                // 如果没有指定type，默认使用文本内容
                task.slave_ids = parseSlaveIds(content);
            }
        }

        // 解析func
        QDomElement funcElem = configElem.firstChildElement("func");
        if (!funcElem.isNull()) {
            QString funcStr = funcElem.text().trimmed();
            if (funcStr.startsWith("0x", Qt::CaseInsensitive)) {
                task.func = static_cast<uint8_t>(funcStr.mid(2).toUInt(nullptr, 16));
            } else {
                task.func = static_cast<uint8_t>(funcStr.toUInt());
            }
        }

        // 解析address
        QDomElement addrElem = configElem.firstChildElement("address");
        if (!addrElem.isNull()) {
            QString addrStr = addrElem.text().trimmed();
            if (addrStr.startsWith("0x", Qt::CaseInsensitive)) {
                task.address = static_cast<uint16_t>(addrStr.mid(2).toUInt(nullptr, 16));
            } else {
                task.address = static_cast<uint16_t>(addrStr.toUInt());
            }
        }

        // 解析write_buffers：处理多个步骤（step属性）
        QDomNodeList writeBufferNodes = configElem.elementsByTagName("write_buffer");
        QMap<int, QByteArray> stepBuffers; // 使用QMap按step排序

        for (int j = 0; j < writeBufferNodes.count(); j++) {
            QDomElement bufferElem = writeBufferNodes.at(j).toElement();
            if (bufferElem.isNull()) continue;

            // 获取step属性（默认1）
            int step = 1;
            if (bufferElem.hasAttribute("step")) {
                step = bufferElem.attribute("step").toInt();
            }

            // 解析缓冲区内容
            QString bufferContent = bufferElem.text().trimmed();
            QByteArray bufferData = parseHexString(bufferContent);

            if (!bufferData.isEmpty()) {
                stepBuffers.insert(step, bufferData);
            }
        }

        // 按step顺序存储到task.write_buffers
        QList<int> steps = stepBuffers.keys();
        std::sort(steps.begin(), steps.end());
        for (int step : steps) {
            task.write_buffers.append(stepBuffers[step]);
        }

        // 解析read_length
        QDomElement readLenElem = configElem.firstChildElement("read_length");
        if (!readLenElem.isNull()) {
            task.read_length = readLenElem.text().toInt();
        } else {
            task.read_length = 0; // 0表示写操作
        }

        tasks.append(task);
    }

    return tasks;
}

QList<TaskInfo> ModbusConfigParser::parseConfig(const QString& filePath, const QList<uint8_t>& gtSlaveIds)
{
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "文件不存在:" << filePath;
        return QList<TaskInfo>();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        return QList<TaskInfo>();
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;

    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        qWarning() << "XML解析错误:" << errorMsg << "行:" << errorLine << "列:" << errorColumn;
        file.close();
        return QList<TaskInfo>();
    }

    file.close();

    qDebug() << "成功从文件加载XML配置:" << filePath;
    return parseConfigImpl(doc, gtSlaveIds);
}

QList<TaskInfo> ModbusConfigParser::parseConfigFromString(const QString& xmlContent, const QList<uint8_t>& gtSlaveIds)
{
    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;

    if (!doc.setContent(xmlContent, &errorMsg, &errorLine, &errorColumn)) {
        qWarning() << "XML解析错误:" << errorMsg << "行:" << errorLine << "列:" << errorColumn;
        return QList<TaskInfo>();
    }

    qDebug() << "成功从字符串加载XML配置";
    return parseConfigImpl(doc, gtSlaveIds);
}
