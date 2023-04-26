#include "sendfileop.h"
#include "httpget.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <ctype.h>
#include <time.h>
#include <iostream>
#include <sstream>
using namespace std;
//#pragma execution_character_set("utf-8")

quint16 PORT; /* TCP port */
qint64 LOADBYTES = 1 * 1024; // every time send 1 kilo-byte(can be changed)
const int DATA_STREAM_VERSION = QDataStream::Qt_5_9;
QString IP;
unsigned int e, n;

/* Initialize & main functions */
sendfileop::sendfileop(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setWindowTitle("send file");
    ui.stLabel->setText("Please select a file to send.");
    /* Initialize socket, Initialize sending file*/
    m_TcpSocket = new QTcpSocket(this);
    fileBytes = sentBytes = restBytes = 0;
    loadBytes = LOADBYTES;
    m_File = Q_NULLPTR;
    O_File = Q_NULLPTR;
    m_pMyAesCBC1 = nullptr;

    ui.sendProg->setRange(0, 100);
    ui.sendProg->setValue(0); // reset progressbar
    ui.sendBtn->setEnabled(false);
    ui.pauseBtn->setEnabled(false);
    ui.exitBtn->setEnabled(true);
    ui.terminateBtn->setEnabled(false);
    ui.IPinput->setPlaceholderText("IP address");
    ui.speedInput->setPlaceholderText("transmission speed");
    ui.portInput->setPlaceholderText("port number");

    /* connected -> start sending(send header) */
    connect(m_TcpSocket, SIGNAL(connected()), this, SLOT(start_transfer()));
    /* connected -> continue sending */
    connect(m_TcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(continue_transfer(qint64)));
    /* socket error -> error handling */
    connect(m_TcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(show_error(QAbstractSocket::SocketError)));
}

void sendfileop::start_transfer()
{
    ui.selectBtn->setEnabled(false);
    //ui.stLabel->setText(QString("Connecting..."));

    ui.stLabel->setText(QString("Generating AES secret key..."));
    const char *c = "0123456789abcdefghijklmnopqrstuvwxyz";
    key = new char[17];
    memset(key, 0, 17);
    for (int j = 0; j < 16; j++)
    {
        int index = rand() % strlen(c);
        key[j] = c[index];
    }

//    cipher = new int[16];
//    for (int j = 0; j < 16; j++){
//        int value = toascii(key[j]);
//        cipher[j] = Encryption(value, 2997, 20131); // 41 * 491
//    }

    cipher = new int[16];
    ui.stLabel->setText(QString("Connection established! Now finding the public key of the host..."));
    HttpGetKey(IP, e, n);
    for (int j = 0; j < 16; j++){
        int value = toascii(key[j]);
        cipher[j] = Encryption(value, e, n);
    }
    ui.pauseBtn->setEnabled(true);
    ui.terminateBtn->setEnabled(true);

    m_pMyAesCBC1 = new MyAesCBC(16, (unsigned char *)key);
    qDebug() << "keysize:" << strlen(key);
    qDebug() << "key:" << key;
    qDebug() << "cipher:" << cipher[0];
    QString cip[16];
    for(int k = 0; k < 16; k++){
        cip[k] = QString::number(cipher[k], 10);
    }
    qDebug() << "cip0:" << cip[0];
    qDebug() << "cip15:" << cip[15];

    // Read the data to be encrypted
    QFile file1(m_strFileName);
    if (!file1.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "warning", "open file failed");
        ui.terminateBtn->setEnabled(false);
        ui.pauseBtn->setEnabled(false);
         ui.stLabel->setText("Please select a new file to send.");
        return;
    }

    QByteArray srcImage = file1.readAll();
    int length1 = srcImage.size();
    file1.close();


    QByteArray enImage1;
    ui.stLabel->setText(QString("Encrypting file..."));
    m_pMyAesCBC1->OnAesEncrypt(srcImage, length1, enImage1);

    //QString FileName1 = m_strFileName.left(m_strFileName.size() - 3);
    QString FileName1 = m_strFileName;
    FileName1 = FileName1.append(".aes");

    QFile wfile1(FileName1);
    if (!wfile1.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, "warning", "can't enImge");
      return;
    }
    wfile1.write(enImage1);
    wfile1.close();
    //QMessageBox::information(this, "hint", "write engine into " + m_strFileName);

    m_strFileName = FileName1;

    m_File = new QFile(FileName1);

    if (!m_File->open(QFile::ReadOnly))
    {
        ui.stLabel->setText(QString("No file selected!"));
        qDebug() << "*** start_transfer(): File-Open-Error";
        return;
    }
    fileBytes = m_File->size();
    ui.sendProg->setValue(0);

    QByteArray buf;
    QDataStream out(&buf, QIODevice::WriteOnly);
    qDebug() << "1" << endl;
    out.setVersion(DATA_STREAM_VERSION);

    /* fileName */
    QString sfName = m_strFileName.right(m_strFileName.size() - m_strFileName.lastIndexOf('/') - 1);

    /* header = total size + fileName length + fileName */
    out << qint64(0) << qint64(0) << sfName;
    for(int k = 0; k < 16; k++){
        out << cip[k];
    }
    qDebug() << "sfName" << sfName << endl;

    /* data size + the size of header */
    fileBytes += buf.size();

    /* Rewrite the first two length fields of header */
    out.device()->seek(0);
    qDebug() << "fileName length" << (qint64(buf.size()) - 2 * sizeof(qint64)) << endl;

    out << fileBytes << (qint64(buf.size()) - 2 * sizeof(qint64));
    qDebug() << "fileBytes" << fileBytes << endl;
    qDebug() << "fileBytes" << (qint64(buf.size()) - 2 * sizeof(qint64)) << endl;

    /* Send header, calculate remaining size */
    restBytes = fileBytes - m_TcpSocket->write(buf);
    buf.resize(0);
    qDebug() << "restBytes" << restBytes << endl;
    ui.stLabel->setText(QString("Sending %1 ...").arg(m_strFileName.left(m_strFileName.size() - 4)));
}

/*--- continue sending ---*/
void sendfileop::continue_transfer(qint64 sentSize)
{
    int bufsize = 0;
    qDebug() << "continue_transfer" << endl;
    sentBytes += sentSize;

    /* sending not finish */
    if (restBytes > 0)
    {
        /* read data from file */
        QByteArray buf = m_File->read(qMin(loadBytes, restBytes));
        /* send */
        bufsize = m_TcpSocket->write(buf);
        qDebug() << "bufsize" << bufsize << endl;

        restBytes -= bufsize;
        ui.sendProg->setValue((fileBytes - restBytes) * 100 / fileBytes);
        buf.resize(0);
        qDebug() << "restBytes" << restBytes << endl;
        //m_File->flush();
    }
    else{
        m_File->close();
    }
    qDebug() << "fileBytes" << fileBytes << endl;

    /* finish sending */
    if (restBytes == 0)
    {
        ui.sendProg->setValue((fileBytes - restBytes) * 100 / fileBytes);
        QMessageBox::information(this, "successful transfer", QString("Finish sending %1 !").arg(m_strFileName.left(m_strFileName.size() - 4)));
        m_strFileName.clear(); // clear file
        ui.stLabel->setText("Please select a new file to send.");
        ui.sendProg->setValue(0);
        ui.pauseBtn->setEnabled(false);
        ui.terminateBtn->setEnabled(false);
        ui.selectBtn->setEnabled(true);
        m_File->remove();
        m_TcpSocket->close(); // close socket
    }
    else ui.sendProg->setValue((fileBytes - restBytes) * 100 / fileBytes);
}

/*--- connect error ---*/
void sendfileop::show_error(QAbstractSocket::SocketError)
{
    qDebug() << "*** Socket Error";
    m_TcpSocket->close();
    ui.stLabel->setText(QString("Fail connection!"));
    ui.sendBtn->setEnabled(true);
    ui.sendProg->reset(); // reset progressbar
    //m_strFileName.clear();
    ui.selectBtn->setEnabled(true);
    ui.terminateBtn->setEnabled(false);
    ui.pauseBtn->setEnabled(false);
}

/*--- select file ---*/
void sendfileop::on_selectBtn_clicked()
{
    QString oldFileName = m_strFileName;
    QDir dir;
    dir.setPath("/storage/emulated/0");
    m_strFileName = QFileDialog::getOpenFileName(this, "Open file", dir.absolutePath());

    if (!m_strFileName.isEmpty())
    {
        ui.stLabel->setText(
            QString("File %1 selected!").arg(m_strFileName));
        ui.sendBtn->setEnabled(true);
    }
    else
    {
        if(oldFileName.isEmpty()){
            ui.stLabel->setText(QString("No file selected!"));
            ui.sendBtn->setEnabled(false);
        }
        else{
            m_strFileName = oldFileName;
            ui.stLabel->setText(
                QString("File %1 selected!").arg(m_strFileName));
            ui.sendBtn->setEnabled(true);
        }

    }
}

/*--- connect ---*/
void sendfileop::on_sendBtn_clicked()
{
    if(ui.portInput->text().isEmpty() || ui.IPinput->text().isEmpty() || ui.speedInput->text().isEmpty()){
        QMessageBox::warning(this, "warning", "Please fill in all the blanks !");
        return;
    }

    QMessageBox msgBox;
    msgBox.setText("Are you sure to transfer this file?");
    //msgBox.setInformativeText("Are you sure to transfer this file?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    int ret = msgBox.exec();
    if(ret == QMessageBox::Yes){
        IP = ui.IPinput->text();
        //IP = "localhost";
        //IP = "192.168.1.100";

        PORT = ui.portInput->text().toFloat();

        int num = 1024;
        loadBytes = num * ui.speedInput->text().toFloat();

        /* send connect request */
        m_bStateFlag = true;
        m_TcpSocket->setProxy(QNetworkProxy::NoProxy);
        ui.stLabel->setText("Trying to connect to the host...");
        m_TcpSocket->connectToHost(QHostAddress(IP), PORT);
        sentBytes = 0;
        //ui.selectBtn->setEnabled(false);
        ui.sendBtn->setEnabled(false);
        ui.exitBtn->setEnabled(true);
        //ui.stLabel->setText(QString("Linking..."));
    }
}

/*--- pause & continue ---*/
void sendfileop::on_pauseBtn_clicked()
{
    if (m_bStateFlag) {
        m_bStateFlag = false;
        disconnect(m_TcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(continue_transfer(qint64)));

        ui.stLabel->setText(QString("Pause transferring %1").arg(m_strFileName.left(m_strFileName.size() - 4)));
        ui.pauseBtn->setText(QString("continue transfer"));
        ui.selectBtn->setEnabled(false);
    }
    else {
        m_bStateFlag = true;
        connect(m_TcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(continue_transfer(qint64)));

        int bufsize = 0;
        QByteArray buf = m_File->read(qMin(loadBytes, restBytes));
        bufsize = m_TcpSocket->write(buf);
        qDebug() << "bufsize" << bufsize << endl;

        restBytes -= bufsize;
        ui.sendProg->setValue((fileBytes - restBytes) * 100 / fileBytes);
        buf.resize(0);
        qDebug() << "restBytes" << restBytes << endl;
        qDebug() << "****continue";
        ui.stLabel->setText(QString("Continue transferring %1").arg(m_strFileName.left(m_strFileName.size() - 4)));
        ui.pauseBtn->setText(QString("pause transfer"));
        ui.selectBtn->setEnabled(false);
    }
}

/*--- exit ---*/
void sendfileop::on_exitBtn_clicked()
{
    m_TcpSocket->close(); // close socket
    m_File->remove();
    close();
}

void sendfileop::on_terminateBtn_clicked()
{
    //ui.sendProg->setValue((fileBytes - restBytes) * 100 / fileBytes);
    m_TcpSocket->close(); // close socket
    m_strFileName.clear(); // clear file
    m_File->remove();
    ui.sendProg->setValue(0);
    QMessageBox::information(this, "successful termination", QString("Terminate sending %1 !").arg(m_strFileName.left(m_strFileName.size() - 4)));
    ui.stLabel->setText("Please select a new file to send.");
    ui.pauseBtn->setEnabled(false);
    ui.selectBtn->setEnabled(true);
    ui.sendBtn->setEnabled(false);
    ui.terminateBtn->setEnabled(false);
    connect(m_TcpSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(continue_transfer(qint64)));
    ui.sendProg->setValue(0);
}



