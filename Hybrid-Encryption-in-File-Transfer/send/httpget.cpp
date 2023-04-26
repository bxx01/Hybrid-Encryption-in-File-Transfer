#include "httpget.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <algorithm>
#include <qmetatype.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QVector>
#include <QNetworkInterface>
class QString;

void HttpGetKey(QString ip, unsigned int &e, unsigned int &n){
    // 构建及发送请求
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QString url = "http://34.92.146.187:8097/questionRecord/getPublickeyInfo/";
    url.append("?ip=" + ip);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

    qDebug()<< "recv info " << url;
    QNetworkReply* reply = manager->get(request);//发起post请求

//    QNetworkReply *pReply = manager->get(request);
    // 开启一个局部的事件循环，等待页面响应结束
    QEventLoop eventLoop;
    QObject::connect(manager, &QNetworkAccessManager::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
    // 获取网页Body中的内容

    //错误处理
    if (reply->error() == QNetworkReply::NoError)
    {
        qDebug() << "request protobufHttp NoError";
    }
    else
    {
        qDebug()<<"request protobufHttp handle errors here";
        QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        //statusCodeV是HTTP服务器的相应码，reply->error()是Qt定义的错误码，可以参考QT的文档
        qDebug( "request protobufHttp found error ....code: %d %d\n", statusCodeV.toInt(), (int)reply->error());
        qDebug(qPrintable(reply->errorString()));
    }

    //请求收到的结果
    QByteArray responseByte = reply->readAll();
    qDebug() << "return: " <<  QString::fromStdString(responseByte.toStdString());
    QJsonParseError json_error;
    QJsonDocument doucment = QJsonDocument::fromJson(responseByte, &json_error);
    if (json_error.error == QJsonParseError::NoError) {
        if (doucment.isObject()) {
          const QJsonObject obj = doucment.object();
          qDebug() << obj;
          e = obj.value("e").toDouble();
          n = obj.value("n").toDouble();
        }
    }
}
