#pragma once

#include "ui_recvfileop.h"
#include "myaescbc.h"
#include "rsa.h"
#include <QtWidgets/QWidget>
#include <QAbstractSocket>
#include <iostream>
#include <QDataStream>
#include <QFile>
#include <QDir>
#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QStandardPaths>
#include <QObject>
#include <QFile>
#include <QImage>
#include <QImageReader>

class QFile;
class QDir;
class QString;
class QTcpServer;
class QTcpSocket;
class recvfileop : public QWidget
{
	Q_OBJECT

public:
    recvfileop(QWidget *parent = Q_NULLPTR);

private:
	Ui::recvfileop ui;
	QTcpServer *m_TcpServer;
	QTcpSocket *m_TcpSocket;
	QString m_FileName;
    QString newName;
    QFile *m_File ;
    QFile *new_File;
    char *key;
    int *cipher;
    unsigned int e, d, n;
    MyAesCBC *m_pMyAesCBC1 = nullptr;
    /* Received data, total data, file name length */
	qint64 gotBytes, fileBytes, nameSize;

private slots:
    void accept_connect();                              /* accept connection from sender */
    void receive_file();                                /* receive file */
    void show_error(QAbstractSocket::SocketError);      /* SOCKET error handling */
    //void on_inputButton_clicked();                       /* input path */
    void on_selectBtn_clicked();
};
