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

    void recvMessageCallback( QByteArray &jsonPayload );

signals:
    void requestFinished(QByteArray data);
    void requestFailed(const QString &errorString);

    void sigRequestDataParser(QByteArray);

    //    void errorHappend(  );

private slots:
    void onFinished(QNetworkReply *reply);

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
