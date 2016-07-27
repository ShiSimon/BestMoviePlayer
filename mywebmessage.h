#ifndef MYWEBMESSAGE_H
#define MYWEBMESSAGE_H

#include <QObject>

class MyWebMessage : public QObject
{
    Q_OBJECT
public:
    MyWebMessage(QObject *parent = 0);
    ~MyWebMessage();

public slots:

    //void onCall(QString appid,QString ipaddress);
};

#endif // MYWEBMESSAGE_H
