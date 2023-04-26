#include "recvfileop.h"
#include "httpsend.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <QFileDialog>
#include <QFile>
#include <sstream>

const quint16 PORT = 2022;
const int DATA_STREAM_VERSION = QDataStream::Qt_5_9;
QString path;

recvfileop::recvfileop(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
    setWindowTitle("recv file");

    /* reset progressbar */
	ui.recvProg->setRange(0, 100);
	ui.recvProg->setValue(0);
    ui.savePath->setPlaceholderText("path to save");
	fileBytes = gotBytes = nameSize = 0;
	m_File = Q_NULLPTR;
    new_File = Q_NULLPTR;
    m_TcpSocket = Q_NULLPTR;
    m_pMyAesCBC1 = nullptr;
    key = new char[17];
    memset(key, 0, 17);

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
    //    int a[3] = {0, 0, 0};
    //    int *k = KeyGeneration(a);
    //    qDebug() << "e = " << k[0];
    //    qDebug() << "d = " << k[1];
    //    qDebug() << "n = " << k[2];
    //    e = k[0];
    //    d = k[1];
    //    n = k[2];

    KeyGeneration(e, d, n);
    HttpSendKey(e, n);

	m_TcpServer = new QTcpServer(this);
    /* connect request -> accept connection */
    connect(m_TcpServer, SIGNAL(newConnection()), this, SLOT(accept_connect()));

    if (!m_TcpServer->listen(QHostAddress::Any, PORT))
    {
        std::cerr << "*** Listen to Port Failed ***" << std::endl;
        qDebug() << m_TcpServer->errorString();
        return;
    }
    ui.stLabel->setText(QString("Linking to Port %1").arg(PORT));
}

/*--- accept connection ---*/
void recvfileop::accept_connect()
{
    qDebug() << "receive_file 1";
	m_TcpSocket = m_TcpServer->nextPendingConnection();
    connect(m_TcpSocket, SIGNAL(readyRead()), this, SLOT(receive_file()));

    /* socket error -> error handling */
    connect(m_TcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(show_error(QAbstractSocket::SocketError)));
    ui.stLabel->setText(QString("Waiting for receiving..."));
	gotBytes = 0;
}

/*--- receive file ---*/
void recvfileop::receive_file()
{
    ui.stLabel->setText(QString("Receiving %1 ...").arg(m_FileName.left(m_FileName.size() - 4)));
    path = ui.savePath->text();
	QDataStream in(m_TcpSocket);
	in.setVersion(DATA_STREAM_VERSION);

    if(gotBytes <= sizeof(qint64) * 2){
        qDebug() << "check0";
        if((m_TcpSocket->bytesAvailable() >= sizeof(qint64) * 2) && (nameSize == 0)){
            qDebug() << "check1";
            in >> fileBytes >> nameSize;
            gotBytes += 2 * sizeof(qint64);
        }
        if((m_TcpSocket->bytesAvailable() >= nameSize) && (nameSize != 0)){
            qDebug() << "check2";
            in >> m_FileName;

            QString cip[16];
            for(int k = 0; k < 16; k++){
                in >> cip[k];
            }

            qDebug() << "cip0:" << cip[0];
            qDebug() << "cip15:" << cip[15];

            cipher = new int[16];
            for(int k = 0; k < 16; k++){
                cipher[k] = cip[k].toInt();
            }
            qDebug() << "cip0:" << cipher[0];

            for(int j = 0; j < 16; j++){
                key[j] = Decryption(cipher[j], d, n);
            }
            qDebug() << "key:" << key;
            qDebug() << "cipher:" << cipher[0];

            gotBytes += nameSize;

            //if file dir without '/'
            if(path.at(path.size() - 1) !='/'){
                path.append("/");
            }
            m_File = new QFile(path + m_FileName);
            if (!m_File->open(QIODevice::WriteOnly)) // open failed
            {
                std::cerr << "*** File Open Failed ***" << std::endl;
                return;
            }
        }
        else return;
    }

    if(gotBytes < fileBytes)
    {
        qDebug() << "check";
        gotBytes += m_TcpSocket->bytesAvailable();
        m_File->write(m_TcpSocket->readAll());

    }

	qDebug() << "fileBytes = " << fileBytes << endl;
    qDebug() << "gotBytes = " << gotBytes << endl;
	ui.recvProg->setValue(gotBytes * 100 / fileBytes);
    qDebug() << "receive_file m_FileName = " << m_FileName << endl;

    if(gotBytes == fileBytes){
        m_pMyAesCBC1 = new MyAesCBC(16, (unsigned char *)key);

        m_File->close(); // close file

        //if file dir without '/'
        if(path.at(path.size() - 1) !='/'){
            path.append("/");
        }

        QFile file(path + m_FileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, "warning", "open file failed");
            return;
        }

        QByteArray enImage = file.readAll();
        int length = enImage.size();

        QByteArray deImage;
        m_pMyAesCBC1->OnAesUncrypt(enImage, length, deImage);

        qDebug() << "m_FileName = " << m_FileName << endl;
        QString FileName1= m_FileName.left(m_FileName.size() - 4);
        qDebug() << "fileName = " << FileName1 << endl;

        //if file dir without '/'
        if(path.at(path.size() - 1) !='/'){
            path.append("/");
        }
        QFile rfile(path + FileName1);
        if (!rfile.open(QIODevice::WriteOnly))
        {
            QMessageBox::warning(this, "warning", "can't write deImage");
            return;
        }

        file.close();
        file.remove();

        rfile.write(deImage);
        rfile.close();

        QMessageBox::information(this, "successful transfer", QString("Finish receiving %1").arg(FileName1));
        //ui.stLabel->setText(QString("Finish receiving %1").arg(FileName1));
    }
}


/*--- connect error ---*/
void recvfileop::show_error(QAbstractSocket::SocketError)
{
    if(ui.recvProg->value() != 100){
        QMessageBox::warning(this, "warning", "transmission interrupted!");
        qDebug() << "path = " << path << endl;
        qDebug() << "m_FileName = " << m_FileName << endl;
        QFile file(path + m_FileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, "warning", "open file failed");
            return;
        }
        m_File->close(); // close file
        file.close();
        file.remove();
    }
	std::cerr << "*** Socket Error ***" << std::endl;
    qDebug() << m_TcpSocket->errorString();
    m_TcpSocket->close(); // socket
	m_TcpSocket = Q_NULLPTR;
	m_File = Q_NULLPTR;
    m_FileName.clear(); // clear fileName
	fileBytes = gotBytes = nameSize = 0;

    ui.recvProg->reset(); // reset progressbar
    ui.stLabel->setText(QString("Waiting for a new transmission..."));
    //close();
}

/*---input file save path --
void recvfileop::on_inputButton_clicked()
{
    path = ui.savePath->text();
}
*/

/*--- select file ---*/
void recvfileop::on_selectBtn_clicked()
{
    QDir dir;
    dir.setPath("/storage/emulated/0");
    path = ui.savePath->text();
    path = QFileDialog::getExistingDirectory(this, "select path", dir.absolutePath());

    if(!path.isEmpty()){
        //if file dir without '/'
        if(path.at(path.size() - 1) !='/'){
            path.append("/");
        }
        ui.stLabel->setText(QString("Path %1 selected!").arg(path));
        ui.savePath->setText(QString("%1").arg(path));
    }
    else{
        if(ui.savePath->text().isEmpty()){
            ui.stLabel->setText(QString("No path selected!"));
        }
        ui.stLabel->setText(QString("Path %1 selected!").arg(ui.savePath->text()));
    }
}
