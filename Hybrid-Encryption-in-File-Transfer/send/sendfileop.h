#pragma once

#include "ui_sendfileop.h"
#include "myaescbc.h"
#include "rsa.h"
#include <QtWidgets/QWidget>
#include <QAbstractSocket>
#include <QByteArray>
#include <QDataStream>
#include <QFileDialog>
#include <QHostAddress>
#include <QIODevice>
#include <QString>
#include <QTcpSocket>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QStandardPaths>
#include <QObject>
#include <QFile>
#include <QImage>
#include <QImageReader>

class QByteArray;
class QFile;
class QString;
class QTcpSocket;
class sendfileop : public QWidget
{
	Q_OBJECT

public:
	sendfileop(QWidget *parent = Q_NULLPTR);

private:
	Ui::sendfileop ui;
    /* TCPsocket, file, fileName */
	QTcpSocket *m_TcpSocket;
	QFile *m_File;
    QFile *O_File;
    QString m_strFileName;
    char* key;
    int *cipher;
    MyAesCBC *m_pMyAesCBC1 = nullptr;

    /* Total data size, sent data size, remaining data size, data block size sent each time, file offset */
	qint64 fileBytes, sentBytes, restBytes, loadBytes, fileoffset;
	bool m_bStateFlag;

private slots:

    void start_transfer();							/* Start to transfer the file, transfer the file header, including file size, file name size, file name */
    void continue_transfer(qint64);					/* continue transfer file itself */
    void show_error(QAbstractSocket::SocketError);	/* SOCKET error handling */
    void on_selectBtn_clicked();					/* select button slot function */
    void on_sendBtn_clicked();						/* send button slot function  */
    void on_pauseBtn_clicked();						/* pause button slot function  */
    void on_exitBtn_clicked();                      /* exit button slot function  */
    void on_terminateBtn_clicked();                 /* terminate button slot function  */
};
