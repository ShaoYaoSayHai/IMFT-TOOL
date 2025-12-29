/**
 * @brief FileRW::FileRW
 * @param parent
 */

#include "filerw.h"
/**
 * @brief 读取XML文件并解析设备信息到结构体列表
 * @param filePath XML文件路径
 * @return 包含所有设备信息的QList
 */
QList<CLTDeviceInfo> readXmlToStruct(QString filePath) {
    QList<CLTDeviceInfo> devices;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << file.errorString();
        return devices; // 返回空列表
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qDebug() << "解析XML文档出错。";
        file.close();
        return devices; // 返回空列表
    }
    file.close();

    // 获取根元素
    QDomElement root = doc.documentElement();
    if (root.isNull()) {
        qDebug() << "无效的XML根节点。";
        return devices;
    }

    // 找到名为 "ParentDeviceNode" 的元素
    QDomElement parentElement = root.firstChildElement("ParentDeviceNode");
    if (parentElement.isNull()) {
        qDebug() << "未找到ParentDeviceNode节点。";
        return devices;
    }

    // 获取所有DeviceNode节点
    QDomNodeList deviceNodes = parentElement.elementsByTagName("DeviceNode");
    //  qDebug() << "找到设备节点数量:" << deviceNodes.count();

    // 遍历所有设备节点
    for (int i = 0; i < deviceNodes.count(); ++i) {
        QDomElement deviceElement = deviceNodes.at(i).toElement();
        if (deviceElement.isNull()) {
            continue;
        }

        // 读取DeviceAddress
        QDomElement addressElement =
                deviceElement.firstChildElement("DeviceAddress");
        // 读取DeviceType
        QDomElement typeElement = deviceElement.firstChildElement("DeviceType");

        if (!addressElement.isNull() && !typeElement.isNull()) {
            QString deviceAddress = addressElement.text();
            QString deviceType = typeElement.text();

            // 使用聚合初始化创建结构体实例并添加到列表
            devices.append({deviceAddress, deviceType});

            //      qDebug() << "成功读取设备 - 地址:" << deviceAddress
            //               << "类型:" << deviceType;
        } else {
            qDebug() << "设备节点" << i + 1 << "缺少地址或类型信息";
        }
    }

    //  qDebug() << "总共读取了" << devices.size() << "个设备的信息";
    return devices;
}

bool readSwitchTimesWithDebug(const QString &filePath, int &switchOpenTime,
                              int &switchCloseTime, int &switchResetTime) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[错误] 无法打开文件:" << file.errorString();
        return false;
    }
    // 可选：将文件内容读入字符串，便于查看原始数据
    QTextStream in(&file);
    QString fileContent = in.readAll();
    file.seek(0); // 将文件指针重置回开头
    QDomDocument doc;
    QString errorMsg;
    int errorLine = 0, errorColumn = 0;
    // 使用带错误信息的解析方法
    if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "[错误] XML解析失败! 行:" << errorLine << "列:" << errorColumn
                 << "错误:" << errorMsg;
        file.close();
        return false;
    }
    file.close();
    //  qDebug() << "[信息] XML解析成功.";

    // 获取根元素
    QDomElement root = doc.documentElement();
    if (root.isNull()) {
        qDebug() << "[错误] 未找到根元素（Root）。";
        return false;
    }
    //  qDebug() << "[信息] 根元素名称:" << root.tagName();

    // 查找 DeviceSwitchConfig 节点
    QDomElement switchConfig = root.firstChildElement("DeviceSwitchConfig");
    if (switchConfig.isNull()) {
        //    qDebug()
        //        << "[错误] 未找到 DeviceSwitchConfig
        //        节点。根元素下的直接子节点有:";
        QDomNodeList childNodes = root.childNodes();
        for (int i = 0; i < childNodes.count(); ++i) {
            QDomNode node = childNodes.at(i);
            if (node.isElement()) {
                qDebug() << "  -" << node.toElement().tagName();
            }
        }
        return false;
    }
    // 读取 SwitchOpenTime
    QDomElement openTimeElement =
            switchConfig.firstChildElement("SwitchOpenTime");
    if (openTimeElement.isNull()) {
        qDebug() << "[错误] 在 DeviceSwitchConfig 下未找到 SwitchOpenTime 节点。";
        return false;
    }
    switchOpenTime = openTimeElement.text().toInt();
    // 读取 SwitchCloseTime
    QDomElement closeTimeElement =
            switchConfig.firstChildElement("SwitchCloseTime");
    if (closeTimeElement.isNull()) {
        //    qDebug() << "[错误] 在 DeviceSwitchConfig 下未找到 SwitchCloseTime
        //    节点。";
        return false;
    }
    switchCloseTime = closeTimeElement.text().toInt();
    //  qDebug() << "[信息] SwitchCloseTime 内容:" << closeTimeElement.text();

    // 读取 SwitchResetTime
    QDomElement resetTimeElement =
            switchConfig.firstChildElement("SwitchResetTime");
    if (resetTimeElement.isNull()) {
        qDebug() << "[错误] 在 DeviceSwitchConfig 下未找到 SwitchResetTime节点。";
        return false;
    }
    switchResetTime = resetTimeElement.text().toInt();
    //  qDebug() << "[信息] SwitchResetTime 内容:" << resetTimeElement.text();
    return true;
}

bool readPressureTimeConfig(const QString &filePath, int &readAirPressureTime,
                            int &readFaqianPressureTime,
                            int &readFahouPressureTime) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[错误] 无法打开文件:" << file.errorString();
        return false;
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine = 0, errorColumn = 0;
    if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "[错误] XML解析失败! 行:" << errorLine << "列:" << errorColumn
                 << "错误:" << errorMsg;
        file.close();
        return false;
    }
    file.close();
    QDomElement root = doc.documentElement();
    if (root.isNull()) {
        qDebug() << "[错误] 未找到根元素（Root）。";
        return false;
    }
    QDomElement timeConfig = root.firstChildElement("ReadPressureTimeConfig");
    if (timeConfig.isNull()) {
        qDebug() << "[错误] 未找到 ReadPressureTimeConfig 节点。";
        return false;
    }
    QDomElement airEl = timeConfig.firstChildElement("ReadAirPressureTime");
    QDomElement faqianEl = timeConfig.firstChildElement("ReadFaqianPressureTime");
    QDomElement fahouEl = timeConfig.firstChildElement("ReadFahouPressureTime");
    if (airEl.isNull() || faqianEl.isNull() || fahouEl.isNull()) {
        qDebug() << "[错误] ReadPressureTimeConfig 子节点不完整。";
        return false;
    }
    readAirPressureTime = airEl.text().trimmed().toInt();
    readFaqianPressureTime = faqianEl.text().trimmed().toInt();
    readFahouPressureTime = fahouEl.text().trimmed().toInt();
    return true;
}

bool readInputControlSwitch(const QString &filePath, uint8_t &a1Addr,
                            uint8_t &a2Addr) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[错误] 无法打开文件:" << file.errorString();
        return false;
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine = 0, errorColumn = 0;
    if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "[错误] XML解析失败! 行:" << errorLine << "列:" << errorColumn
                 << "错误:" << errorMsg;
        file.close();
        return false;
    }
    file.close();
    QDomElement root = doc.documentElement();
    if (root.isNull()) {
        qDebug() << "[错误] 未找到根元素（Root）。";
        return false;
    }
    QDomElement inputSwitch = root.firstChildElement("InputControlSwitch");
    if (inputSwitch.isNull()) {
        qDebug() << "[错误] 未找到 InputControlSwitch 节点。";
        return false;
    }
    QDomElement a1El = inputSwitch.firstChildElement("InputControlSwitchA1");
    QDomElement a2El = inputSwitch.firstChildElement("InputControlSwitchA2");
    if (a1El.isNull() || a2El.isNull()) {
        qDebug() << "[错误] 缺少 A1 或 A2 子节点。";
        return false;
    }
    QDomElement a1AddrEl = a1El.firstChildElement("SwitchAddress");
    QDomElement a1TypeEl = a1El.firstChildElement("SwitchType");
    QDomElement a2AddrEl = a2El.firstChildElement("SwitchAddress");
    QDomElement a2TypeEl = a2El.firstChildElement("SwitchType");
    if (a1AddrEl.isNull() || a1TypeEl.isNull() || a2AddrEl.isNull() ||
            a2TypeEl.isNull()) {
        qDebug() << "[错误] A1/A2 子节点内容不完整。";
        return false;
    }
    QString a1Address = a1AddrEl.text().trimmed();
    //  a1Type = a1TypeEl.text().trimmed();
    QString a2Address = a2AddrEl.text().trimmed();
    //  a2Type = a2TypeEl.text().trimmed();

    a1Addr = a1Address.toUInt(nullptr, 16);
    //   uint8_t a1TypeVal = a1Type.toUInt(nullptr, 16);
    a2Addr = a2Address.toUInt(nullptr, 16);
    //   uint8_t a2TypeVal = a2Type.toUInt(nullptr, 16);
    return true;
}

bool readPressureTimeoutConfig(const QString &filePath, int &readTimeout1,
                               int &readTimeout2) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[错误] 无法打开文件:" << file.errorString();
        return false;
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine = 0, errorColumn = 0;
    if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "[错误] XML解析失败! 行:" << errorLine << "列:" << errorColumn
                 << "错误:" << errorMsg;
        file.close();
        return false;
    }
    file.close();
    QDomElement root = doc.documentElement();
    if (root.isNull()) {
        qDebug() << "[错误] 未找到根元素（Root）。";
        return false;
    }
    QDomElement timeConfig = root.firstChildElement("ReadPressureTimeConfig");
    if (timeConfig.isNull()) {
        qDebug() << "[错误] 未找到 ReadPressureTimeConfig 节点。";
        return false;
    }
    QDomElement lowEl = timeConfig.firstChildElement("ReadLowPressureTimeout");
    QDomElement overEl = timeConfig.firstChildElement("ReadOverPressureTimeout");
    if (lowEl.isNull() || overEl.isNull()) {
        qDebug() << "[错误] Timeout 节点不完整。";
        return false;
    }
    readTimeout1 = lowEl.text().trimmed().toInt();
    readTimeout2 = overEl.text().trimmed().toInt();
    return true;
}

/**
 * @brief readInternetMesConfigInfo 读取配置信息
 * @param filePath 路径
 * @param part 目标内容
 * @return
 */
QString readInternetMesConfigInfo(const QString &filePath, QString part) {
    QString ipAddress = "";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[错误] 无法打开文件:" << file.errorString();
        return nullptr;
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine = 0, errorColumn = 0;
    if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "[错误] XML解析失败! 行:" << errorLine << "列:" << errorColumn
                 << "错误:" << errorMsg;
        file.close();
        return nullptr;
    }
    file.close();
    QDomElement root = doc.documentElement();
    if (root.isNull()) {
        qDebug() << "[错误] 未找到根元素（Root）。";
        return nullptr;
    }

    QDomElement timeConfig = root.firstChildElement("MesConfig");
    if (timeConfig.isNull()) {
        qDebug() << "[错误] 未找到 MesConfig 节点。";
        return "";
    }

    QDomElement info = timeConfig.firstChildElement(part);
    if (info.isNull()) {
        qDebug() << "[错误] Timeout 节点不完整。";
        return "";
    }
    return info.text();
}

bool FileTakeRootElement(const QString& path, QDomElement& rootElement, QString& errorMsg, int* errorLine, int* errorColumn)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMsg = QString("无法打开文件: %1").arg(file.errorString());
        return false;
    }
    QDomDocument doc;
    if (!doc.setContent(&file, false, &errorMsg, errorLine, errorColumn)) {
        file.close();
        return false;
    }
    file.close();

    rootElement = doc.documentElement();
    if (rootElement.isNull()) {
        errorMsg = "未找到根元素（Root）。";
        return false;
    }

    // 关键：让doc离开作用域，但rootElement仍然有效
    // 因为rootElement现在引用的是调用方维护的QDomDocument中的元素
    return true;
}

bool read_xml_cmd( QString path )
{
    QDomElement root;
    QString errorMsg ;
    int errorLine , errorColumn;
    bool ret = FileTakeRootElement( path , root , errorMsg , &errorLine , &errorColumn );
    if( ret == false )
    { return false;  }
    // 读取ROOT根节点成功 BuildUsrOpenFireFunc
    QDomElement buildCommand = root.firstChildElement("BuildUsrOpenFireFunc");
    if (buildCommand.isNull()) {
        //      qDebug() << "[错误] 未找到 BuildUsrOpenFireFunc 节点。";
        return false ;
    }
    // 查找某个节点
    QDomElement info = buildCommand.firstChildElement( "commandOpenInputValve" );
    if (info.isNull()) {
        //      qDebug() << "[错误] OpenAirValveBeforeFireCommand 节点不完整。";
        return false;
    }
    // 查找某个节点
    QDomElement childInfo = info.firstChildElement( "enable" );
    if (info.isNull()) {
        //      qDebug() << "[错误] enable 节点不完整。";
        return false ;
    }
    qDebug()<<"childInfo-text"<<childInfo.text() ;
    return childInfo.text().contains("true")?true:false;
}


// 解析slave_id的辅助函数
QList<quint8> parseSlaveId(const QString& text, const QDomElement& element) {
    QList<quint8> slaveList;

    QString useSlaveId = element.attribute("use_slaveid", "false");
    if (useSlaveId.compare("true", Qt::CaseInsensitive) != 0) {
        return slaveList;
    }

    if (text.isEmpty()) return slaveList;

    // 处理可能的格式问题（如0xA40xA5变为0xA4,0xA5）
    QString processedText = text;
    processedText.replace(QRegExp("(0x[0-9A-Fa-f]+)(0x)"), "\\1,\\2");

    QStringList idStrings = processedText.split(',', Qt::SkipEmptyParts);
    for (const QString& idStr : idStrings) {
        QString trimmedStr = idStr.trimmed();
        if (!trimmedStr.isEmpty()) {
            bool ok;
            quint8 idValue = trimmedStr.toUInt(&ok, 16);
            if (ok) {
                slaveList.append(idValue);
            }
        }
    }
    return slaveList;
}



// 解析write_buffer的辅助函数
QByteArray parseWriteBuffer(const QString& text, const QDomElement& element) {
    //    QByteArray buffer;
    //    if (text.isEmpty()) return buffer;

    //    QString cleanText = text;
    //    cleanText.remove(QRegExp("\\s")); // 移除空白字符
    //    uint8_t ucCleanValue = cleanText.toUInt() & 0xFF ;
    //    buffer = QByteArray::fromHex( QByteArray::number( ucCleanValue ) );

    //    return buffer;

    QByteArray buffer;

    if (text.isEmpty()) {
        return buffer;
    }

    // 获取size属性，默认为1（兼容旧格式）
    int expectedSize = 1;
    if (element.hasAttribute("size")) {
        bool ok;
        expectedSize = element.attribute("size").toInt(&ok);
        if (!ok || expectedSize <= 0) {
            qWarning() << "Invalid size attribute, using default value 1";
            expectedSize = 1;
        }
    }

    // 预处理文本：移除所有空白字符和"0x"前缀
    QString processedText = text;
    processedText.remove(QRegExp("\\s")); // 移除空白字符（空格、换行等）
    processedText.remove(QRegExp("0x"));  // 移除十六进制前缀"0x"
    processedText.remove(',');             // 移除逗号分隔符

    // 检查处理后的字符串长度是否为偶数（有效的十六进制字符串）
    if (processedText.length() % 2 != 0) {
        qWarning() << "Invalid hex string length after preprocessing:" << processedText;
        return buffer;
    }

    // 将十六进制字符串转换为QByteArray
    buffer = QByteArray::fromHex(processedText.toLatin1());

    // 验证解析后的数据长度是否符合size属性预期
    if (buffer.size() != expectedSize) {
        qWarning() << "Parsed data size" << buffer.size()
                   << "does not match expected size" << expectedSize;
        // 可以根据需要截断或填充数据，这里直接返回解析结果
    }

    return buffer;
}

// 核心解析函数 - 解析单个命令元素
bool parseCommandElement(const QDomElement& element, CommandParams& params) {
    params.commandType = element.tagName();

    QDomNode child = element.firstChild();
    while (!child.isNull()) {
        if (child.isElement()) {
            QDomElement childElem = child.toElement();
            QString tagName = childElem.tagName();
            QString text = childElem.text().trimmed();

            if (tagName == "id") {
                params.id = text.toInt();
            } else if (tagName == "enable") {
                params.enable = (text.compare("true", Qt::CaseInsensitive) == 0);
            } else if (tagName == "slave_id") {
                params.slave_id = parseSlaveId(text, childElem);
            } else if (tagName == "func") {
                params.func = text.toUInt(nullptr, 16);
            } else if (tagName == "address") {
                params.address = text.toUInt(nullptr, 16);
            } else if (tagName == "write_buffer") {
                params.write_buffer = parseWriteBuffer(text , childElem);
            } else if (tagName == "read_length") {
                params.read_length = text.toInt();
            }
        }
        child = child.nextSibling();
    }
    return true;
}

// 主解析函数 - 遍历所有命令并排序
QList<CommandParams> parseAllCommands(const QDomElement& rootElement) {
    QList<CommandParams> commands;

    QDomNode node = rootElement.firstChild();
    while (!node.isNull()) {
        if (node.isComment()) {
            node = node.nextSibling();
            continue;
        }

        if (node.isElement()) {
            CommandParams params;
            if (parseCommandElement(node.toElement(), params)) {
                commands.append(params);
            }
        }
        node = node.nextSibling();
    }

    // 按照id进行排序
    std::sort(commands.begin(), commands.end(),
              [](const CommandParams& a, const CommandParams& b) {
        return a.id < b.id;
    });

    return commands;
}

// 使用示例：从XML文件解析
QList<CommandParams> parseXmlFile(const QString& filePath) {
    QList<CommandParams> commands;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        return commands;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        qWarning() << "无法解析XML文件";
        file.close();
        return commands;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomElement buildFuncElement = root.firstChildElement("BuildUsrOpenFireFunc");
    if (buildFuncElement.isNull()) {
        qWarning() << "未找到BuildUsrOpenFireFunc节点";
        return commands;
    }
    return parseAllCommands(buildFuncElement);
}



QString parseLoginResponse(const QString& jsonResponse) {
    // 第一步：解析外层JSON
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonResponse.toUtf8());
    if (jsonDoc.isNull()) {
        qWarning() << "Failed to parse JSON response.";
        return "Error: Invalid JSON";
    }

    QJsonObject jsonObj = jsonDoc.object();
    if (!jsonObj.contains("d")) {
        qWarning() << "JSON response does not contain 'd' field.";
        return "Error: Missing 'd' field";
    }

    // 获取内层的XML字符串，并处理Unicode转义序列（如\u003c）
    QString innerXmlString = jsonObj["d"].toString();

    // 第二步：准备解析内层XML
    QXmlStreamReader xmlReader(innerXmlString);

    QString retVal;
    QString retMsg;

    // 遍历XML元素
    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        QXmlStreamReader::TokenType token = xmlReader.readNext();

        // 查找开始标签
        if (token == QXmlStreamReader::StartElement) {
            if (xmlReader.name() == "info") {
                // 提取RETVAL和RETMSG属性
                retVal = xmlReader.attributes().value("RETVAL").toString();
                retMsg = xmlReader.attributes().value("RETMSG").toString();
                break; // 找到所需信息后退出循环
            }
        }
    }

    // 检查XML解析是否出错
    if (xmlReader.hasError()) {
        qWarning() << "XML parsing error:" << xmlReader.errorString();
        return "Error: XML parse failed";
    }

    // 第三步：根据RETVAL判断返回结果
    if (retVal == "1") {
        return "PASS"; // 成功时返回PASS
    } else {
        return retMsg; // 失败时返回完整的错误信息
    }
}


bool parseStaIsOmft(const QString& xml, QString* outSta)
{
    QXmlStreamReader reader(xml);

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement() && reader.name() == QLatin1String("info")) {
            const auto attrs = reader.attributes();

            // 关键修正：不要用 u"STA"，改用 QLatin1String 或 QStringLiteral
            const QString sta = attrs.value(QLatin1String("STA")).toString().trimmed();
            if (outSta) *outSta = sta;

            if (sta.isEmpty()) {
                qWarning() << "[XML] STA attribute missing/empty";
                return false;
            }

            const bool ok = (sta == QLatin1String("OMFT"));
            if (ok) {
                qInfo() << "[XML] STA =" << sta << "-> return true";
            } else {
                qWarning() << "[XML] STA =" << sta << "-> return false";
            }
            return ok;
        }
    }

    if (reader.hasError()) {
        qWarning() << "[XML] Parse error:" << reader.errorString();
    } else {
        qWarning() << "[XML] <info> element not found";
    }
    return false;
}
