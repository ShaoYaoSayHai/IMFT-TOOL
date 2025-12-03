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
  qDebug() << "找到设备节点数量:" << deviceNodes.count();

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

      qDebug() << "成功读取设备 - 地址:" << deviceAddress
               << "类型:" << deviceType;
    } else {
      qDebug() << "设备节点" << i + 1 << "缺少地址或类型信息";
    }
  }

  qDebug() << "总共读取了" << devices.size() << "个设备的信息";
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
