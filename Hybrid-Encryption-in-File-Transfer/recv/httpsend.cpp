#include "httpsend.h"
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

QString GetIPS()
{
    QString ip;
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list) {
       if(address.protocol() == QAbstractSocket::IPv4Protocol)
       //我们使用IPv4地址
       {
           ip = address.toString();
           if(ip.startsWith("10.", Qt::CaseSensitive) || ip.startsWith("192.", Qt::CaseSensitive) || ip.startsWith("172.", Qt::CaseSensitive)) {
               break;
           }
           continue;
       }
    }
    qDebug()<< "IP: " << ip;
    return ip;
}

void HttpSendKey(int e, int n){
//    QList<QHostAddress> list = QNetworkInterface::allAddresses();
//     foreach (QHostAddress address, list)
//     {
//          //只留IPv4
//         if(address.protocol() == QAbstractSocket::IPv4Protocol){
//            //只留192.168.1段
//            if( address.toString().contains("192.168.1.")){
//                qDebug()<< address.toString();
//            }

//         }
//    }

    // 构建及发送请求
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QString url = "http://34.92.146.187:8097/questionRecord/setPublickeyInfo/";
    url.append("?ip=" + GetIPS());
    url.append("&e=" + QString::number(e));
    url.append("&n=" + QString::number(n));
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/json"));

//    QJsonObject object;
//    object.insert("id", GetIPS());
//    object.insert("d",d);
//    object.insert("n",n);

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
}
