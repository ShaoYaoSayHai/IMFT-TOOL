#include "httpclient.h"
#include <QNetworkRequest>
#include "infoparse.h"
#include "./FileReadWrite/filerw.h"

HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &HttpClient::onFinished);

    QString ip_address = readInternetMesConfigInfo( "./buildConfig.xml" , "MesIp" );
    QString port = readInternetMesConfigInfo( "./buildConfig.xml" , "MesPort" ) ;
    QString checkPath = readInternetMesConfigInfo( "./buildConfig.xml" , "MesPathCheck" ) ;
    QString updatePath = readInternetMesConfigInfo( "./buildConfig.xml" , "MesPathUpdate" ) ;

//    qDebug()<<"ip : "<<ip_address<<" port : "<<port << " checkPath : "<<checkPath<<" updatePath : "<<updatePath ;
}

HttpClient::~HttpClient()
{
    if (m_manager)
    {
        m_manager->deleteLater();
    }
}

void HttpClient::doGet(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    m_manager->get(request);
//    qDebug() << "Sending login request to:" << url.toString();
}

void HttpClient::doPost(const QUrl &url,
                        const QByteArray &body,
                        const QString &contentType)
{
    QNetworkRequest request(url);
    // request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    m_manager->post(request, body);
    qDebug() << "Sending login request to:" << url.toString();
}

void HttpClient::postMesCheck(const QString &info)
{
    QJsonObject jsonData;
    jsonData["InputXml"] = info;
    QJsonDocument doc(jsonData);
    QByteArray data = doc.toJson();
    this->doPost(QUrl("http://192.168.30.133:6688/WebService1.asmx/MesCheck"),
                 data,
                 "application/json");
}

void HttpClient::postMesUpdate(const QString &info)
{
    QJsonObject jsonData;
    jsonData["InputXml"] = info;
    QJsonDocument doc(jsonData);
    QByteArray data = doc.toJson();
    this->doPost(QUrl("http://192.168.30.133:6688/WebService1.asmx/MesUpdate"),
                 data,
                 "application/json");
}

QByteArray HttpClient::getFinishedCallbackMsg()
{
    return  msg ;
}

/**
 * @brief 拼接出update所需要的信息
 * @param jsonPayload
 */
void HttpClient::recvMessageCallback( QByteArray &jsonPayload )
{
    QMap<QString , QString> map = InfoParser::JsonPayloadParse( jsonPayload ) ;
    if( !map.isEmpty() && map.size() == 3 )
    {
        const QString sn    = map.value(QStringLiteral("SN"));
        const QString imei1 = map.value(QStringLiteral("IMEI1"));
        const QString iccid = map.value(QStringLiteral("ICCID"));
        QString msg = buildInputPayload( map.value(QStringLiteral("SN")) , "OMFT" , map.value("IMEI1") , map.value(QStringLiteral("ICCID")) );
        qDebug()<<"拼接内容 - "<<msg ;
        this->postMesUpdate( msg ); // 更新到最后
    }
}

void HttpClient::onFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        // 读取响应数据
        QByteArray response = reply->readAll();
        QString responseString = QString::fromUtf8(response);
        qDebug() << "Login successful! Response:" << responseString;
        this->msg = responseString.toUtf8() ;
        recvMessageCallback( this->msg ); // 拼接出结果
        // 解析异常
//        QString ret = parseLoginResponse( responseString );
//        qDebug()<<"返回值 : "<<ret ;
        emit requestFinished( response );
    }
    else
    {
        QString errorString = reply->errorString();
        int errorCode = reply->error();
        // 获取HTTP状态码
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid())
        {
            errorString = QString("HTTP %1: %2").arg(statusCode.toInt()).arg(errorString);
        }
        qDebug() << "Login failed! Error:" << errorString << "Error code : "<<errorCode;
    }
    // 清理reply对象
    reply->deleteLater();
}
