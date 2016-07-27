#include "helper.h"

#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <qthread.h>

class Sleeper : public QThread
{
public:
    static void sleep(unsigned long secs){QThread::sleep(secs);}
    static void msleep(unsigned long msecs){
        QThread::msleep(msecs);
    }
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
};

QString Helper::formatTime(int secs){
    bool negative = (secs<0);
    secs = abs(secs);

    int t = secs;
    int hours = (int)t/3600;
    t -= hours*3600;
    int minutes = (int)t/60;
    t -= minutes*60;
    int seconds = t;

    return QString("%1%2:%3:%4").arg(negative?"-":"").arg(hours,2,10,QChar('0')).arg(minutes,2,10,QChar('0'))
            .arg(seconds,2,10,QChar('0'));
}
