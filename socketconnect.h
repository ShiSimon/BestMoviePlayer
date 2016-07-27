#ifndef SOCKETCONNECT_H
#define SOCKETCONNECT_H

#include <QObject>
#include <QObject>
#include <QtNetwork>
#include <QThread>
#include <QString>
#include <inttypes.h>
#include <QString>
#include <QTimer>
#include <qdatastream.h>
#include <QStringList>
#include <list>
#include "tinyxml.h"

class PlayList;

const quint8 start_header[16] = {0x03,0x0e,0x2a,0x34,0x02,0x03,0x01,0x01,0x0f,0x05,0x01,0x10,0x00,0x00,0x00};

class SocketConnect : public QObject
{
    Q_OBJECT
public:
    SocketConnect(QObject *parent = 0);

    PlayList *playlist;

    QString prikeybuffer;

    int parse_CreateSpl(TiXmlDocument *test_doc);
    int parse_SelectSpl(TiXmlDocument *test_doc);
    void parse_setprikeyinfo(TiXmlDocument *test_doc);
    void parse_setserverdate(TiXmlDocument *test_doc);
    void responseGetprikeyInfo(QString appid);
    void responseGetserverTime(QString time);
    void make_response(const char *message,int error);
    void send_response();
    void send_response(TiXmlDocument *status_response);
    void response_getstatus(int status,QString cpl_uuid,qint32 duration);

signals:
//    void socketplay();
    void createspl();
    void selectspl();
    void playspl();
    void stopspl();
    void get_status();
    void getservertime();
    void getprikeyinfo();
    void setprikeyinfo(const char* prikeyvalue);
    void setserverdatetime(QString s);

protected slots:
    void acceptConnection();
    void readMessage();

private:
    QTcpServer TcpSocket;
    quint32 blockSize;
    quint8 header[16];
    QTimer *poller;
    QTcpSocket *tcpclient;
    TiXmlDocument *response;
};

#endif // SOCKETCONNECT_H
