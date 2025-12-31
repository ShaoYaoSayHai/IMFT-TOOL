#include "httpclient.h"
#include <QNetworkRequest>
#include "infoparse.h"
#include "./FileReadWrite/filerw.h"
#include "./FileReadWrite/mes_parse.h"
#include "./FileReadWrite/mes_sn_retmsg_parse.h"

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
void HttpClient::MesCheckCallbackParse( QByteArray &jsonPayload )
{
    QMap<QString , QString> map = InfoParser::JsonPayloadParse( jsonPayload ) ;
    if( !map.isEmpty() && map.size() == 3 )
    {
        const QString sn    = map.value(QStringLiteral("SN"));
        const QString update_info = buildInputXmlBytes( sn , "OMFT" , "PASS" );
        qDebug()<<"UPDATE INFO : "<<update_info ;
        this->postMesUpdate( update_info ); // 更新
    }
}

void HttpClient::error_happen_call_back(QString msg)
{
    if( msg.contains("不匹配") )
    {
        emit requestFinished(msg.toUtf8()) ;
    }
    else
    {
        emit requestFinished("未知错误 : "+msg.toUtf8()) ;
    }
}

void HttpClient::receive_net_results( QByteArray &data )
{
    QString xml, err;
    if( extractXmlFromServerResp( data , &xml , &err ) )
    {
        qDebug().noquote() << "Decoded XML:" << xml;
        emit requestFinished( "MES PASS :: "+xml.toUtf8() );
    }
    else
    {
        qDebug().noquote() << "Decode failed:" << err;
        emit requestFinished( "MES FAIL :: "+err.toUtf8() );
    }
}

void HttpClient::onFinished(QNetworkReply *reply)
{
    // 获取URL
    const QString path = reply->url().path();  // 例如 /WebService1.asmx/MesCheck
    if (reply->error() == QNetworkReply::NoError)
    {
        // 读取响应数据
        QByteArray response = reply->readAll();
        QString responseString = QString::fromUtf8(response);
        this->msg = responseString.toUtf8() ;

        // 2025年12月30日 新增代码
        if (path.endsWith("/MesCheck")) {
            QString errorMsg ;
            if( parseRetmsgPassFromJsonCompat( responseString , &errorMsg ) != false )
            {
                qDebug()<<"JSON解析通过 MES CHECK OK" ;
                MesCheckCallbackParse(this->msg);
                receive_net_results( response );
                MesParseResult pr = parseSnAndRetmsg_CE02Compat(responseString);
                qDebug() << "SN=" << pr.sn << "RETMSG=" << pr.retmsg << "OK=" << pr.ok << "ERR=" << pr.rawError;
                // 信号量，用作修改UI界面
                emit MES_ResultReload( pr.sn , true );
            }
            else
            {
                qDebug()<<"MesCheck Error : "<<errorMsg ;
                error_happen_call_back(errorMsg);
                MesParseResult pr = parseSnAndRetmsg_CE02Compat(responseString);
                qDebug() << "SN=" << pr.sn << "RETMSG=" << pr.retmsg << "OK=" << pr.ok << "ERR=" << pr.rawError;
                // 信号量，用作修改UI界面
                emit MES_ResultReload( pr.sn , false );
            }
        }
        else if (path.endsWith("/MesUpdate")) {
            // 解析 update 返回
//            receive_net_results( response );
            QString SN , retMsg ;
            bool submit_result = parseMesRetmsgAndExtractSn(responseString , &SN , &retMsg  );
            if( submit_result == false )
            {
                emit MES_ResultReload( SN , true ); // 已经不在当前工站，意味着之前提交成功
            }
        }
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
