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
        QDomElement addressElement = deviceElement.firstChildElement("DeviceAddress");
        // 读取DeviceType
        QDomElement typeElement = deviceElement.firstChildElement("DeviceType");

        if (!addressElement.isNull() && !typeElement.isNull()) {
            QString deviceAddress = addressElement.text();
            QString deviceType = typeElement.text();

            // 使用聚合初始化创建结构体实例并添加到列表
            devices.append({deviceAddress, deviceType});

            // 或者使用构造函数方式（如果定义了构造函数）
            // devices.append(DeviceInfo(deviceAddress, deviceType));

            qDebug() << "成功读取设备 - 地址:" << deviceAddress << "类型:" << deviceType;
        } else {
            qDebug() << "设备节点" << i+1 << "缺少地址或类型信息";
        }
    }

    qDebug() << "总共读取了" << devices.size() << "个设备的信息";
    return devices;
}
