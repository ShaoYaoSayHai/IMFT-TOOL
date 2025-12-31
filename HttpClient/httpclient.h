#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <functional>
#include "config/version_config.h"
#include "./infoparse.h"
#include "./FileReadWrite/mes_retmsg_parser.h"

#include <QThread>

class HttpClient : public QObject
{
    Q_OBJECT
public:
    explicit HttpClient(QObject *parent = nullptr);
    ~HttpClient();

    bool updateFlag = false ;

    enum class ReqType {
        Unknown = 0,
        MesCheck,
        MesUpdate,
        GenericGet,
        GenericPost
    };
    Q_ENUM(ReqType)

    // 触发 GET 请求
    void doGet(const QUrl &url);

    // 触发 POST 请求，body 可以是表单或 JSON
    void doPost(const QUrl &url, const QByteArray &body,
                const QString &contentType = QStringLiteral("application/json"));

    void postMesCheck(const QString &body);

    void postMesUpdate(const QString &info);

    QByteArray getFinishedCallbackMsg();

    void MesCheckCallbackParse( QByteArray &jsonPayload );

signals:

#if 1
    void requestFinished(QByteArray data);
#else
    // 修改后结果
    void requestFinished(HttpClient::ReqType type, const QByteArray& response);
#endif
    void requestFailed(const QString &errorString);

    void sigRequestDataParser(QByteArray);

    //    void errorHappend(  );

    void MES_ResultReload( QString sn , bool status );

private slots:
    void onFinished(QNetworkReply *reply);
    void error_happen_call_back(QString msg);
    void receive_net_results(QByteArray &data);
private:
    QNetworkAccessManager *m_manager = nullptr;

    QByteArray msg ;

    // URL设置
    QString url = "http://192.168.30.133" ; // 默认地址
    int port  = 6688 ;// 默认端口号
    QString path_check = "/WebService1.asmx/MesCheck" ; // 读取当前序列号信息路径
    QString path_update = "/WebService1.asmx/MesUpdate" ;

};

#endif // HTTPCLIENT_H
