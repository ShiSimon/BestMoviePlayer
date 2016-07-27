#include "myplayerprocess.h"
#include "config.h"
#include <QRegExp>
#include <QDebug>

MyplayerProcess::MyplayerProcess(QObject *parent)
    :QProcess(parent)
    ,received_end_of_file(false)
{
    resetDcp();
    setProcessChannelMode(QProcess::MergedChannels);
    connect(this,SIGNAL(readyReadStandardOutput()),this,SLOT(readStdOut()));
    connect(this,SIGNAL(lineAvaiable(QByteArray)),this,SLOT(parseLine(QByteArray)));
    connect(this,SIGNAL(finished(int,QProcess::ExitStatus)),
            this,SLOT(procFinished(int,QProcess::ExitStatus)));
}

bool MyplayerProcess::start(){
    received_end_of_file = false;
    remaining_output.clear();
    for(int i = 0;i<arg.length();i++)
    {
//        qDebug()<<arg.at(i);
    }
    QProcess::start(MplayerBin,arg);

    return waitForStarted();
}

void MyplayerProcess::resetDcp(){
    arg.clear();
}

bool MyplayerProcess::isRunning(){
    return (state() == QProcess::Running);
}

void MyplayerProcess::setCommonOptions(){
    arg.clear();
    arg << "-noquiet" << "-slave" << "-identify";
    arg <<"-nokeepaspect";
//    arg <<"-loop"<<"0";
    arg << "-osdlevel"<<"0";
}

void MyplayerProcess::setVolumeOptions(int v){
    arg << "-volume" << QString::number(v);
}

void MyplayerProcess::addAF(const QString &filter_name, const QVariant &value){
    QString s = filter_name;
    if(!value.isNull()) s += "=" + value.toString() + "\n";
    arg << "-af-add" <<s;
}

void MyplayerProcess::setShowscreenOptions(QString winID){
    arg << "-wid" <<winID;
}

void MyplayerProcess::setKeyOptions(QString keyin){
    QString keys="cryptokey=";
    keys = keys + keyin;
    arg<<"-lavfdopts";
    arg << keys;
}

void MyplayerProcess::setDcpOptions(QString video_name, QString audio_name){
    arg << video_name;
    arg << "-audiofile";
    arg << audio_name;
}

void MyplayerProcess::setChannel(int num){
    arg << "-channels";
    arg << QString::number(num);
}

void MyplayerProcess::setStartTime(double sec){
    arg << "-ss";
    arg << QString::number(sec);
}

void MyplayerProcess::setPause(bool b){
    write("pause\n");
}

void MyplayerProcess::seek(double secs, int mode, bool precise){
    QString s = QString("seek %1 %2").arg(secs).arg(mode);
    //if(precise)s += "1"; else s += "-1";
    s += "\n";
    qDebug()<<"seek_cmd="<<s;
    write(s.toLocal8Bit().data());
}

void MyplayerProcess::setAspect(double asp){
    QString s = QString("switch_ratio %1").arg(asp);
    s = s+"\n";
    write(s.toLocal8Bit().data());
}

void MyplayerProcess::setAudioDelay(double delay){
    QString s = pausing_prefix + "audio_delay" + QString::number(delay) + "1\n";
    write(s.toLocal8Bit().data());
}

void MyplayerProcess::quit(){
    qDebug()<<"MplayerProcess::quit";
    write("quit\n");
}

void MyplayerProcess::setVolume(int v){
    QString s = QString("volume %1 1").arg(v);
    s += "\n";
    qDebug()<<s;
    write(s.toLocal8Bit().data());
}

void MyplayerProcess::mute(bool b){
    QString s = pausing_prefix + " mute " + QString::number(b?1:0)+"\n";
    qDebug()<<s;
    write(s.toLocal8Bit().data());
}

void MyplayerProcess::procFinished(int exitCode,QProcess::ExitStatus exitStatus){
    arg.clear();
    qDebug("MyplayerProcess::procFinished");
    emit processExited();
    if(received_end_of_file) emit receivedEndOfFile();
}

static QRegExp rx_av("^[AV]: *([0-9,:.-]+)");
static QRegExp rx_frame("^[AV]:.* (\\d+)\\/.\\d+");// [0-9,.]+");
static QRegExp rx("^(.*)=(.*)");
static QRegExp rx_winresolution("^VO: \\[(.*)\\] (\\d+)x(\\d+) => (\\d+)x(\\d+)");
static QRegExp rx_paused("^ID_PAUSED");
static QRegExp rx_play("^Starting playback...");
static QRegExp rx_endoffile("^Exiting...\\(End of file\\)|^ID_EXIT=EOF");
static QRegExp rx_aspect2("^Movie-Aspect is ([0-9,.]+):1");

void MyplayerProcess::readStdOut()
{
    QByteArray buffer = remaining_output + readAllStandardOutput();
    int start = 0;
    int from_pos = 0;
    int pos = canReadLine(buffer,from_pos);
    while(pos > -1){
        QByteArray line = buffer.mid(start,pos-start);
        from_pos = pos + 1;
        if((from_pos < buffer.size()) && (buffer.at(from_pos)=='\n'))from_pos++;
        start = from_pos;
        emit lineAvaiable(line);

        pos = canReadLine(buffer,from_pos);
    }
    remaining_output = buffer.mid(from_pos);
}

void MyplayerProcess::parseLine(QByteArray ba){
    QString tag;
    QString value;
        //qDebug("%s",ba.data());
    QString line = QString::fromLocal8Bit(ba);
    //Parse A:V Line
    if(rx_av.indexIn(line) > -1){
        double sec = rx_av.cap(1).toDouble();
        //qDebug("MplayerProcess::parseLine:starting sec:%f",sec);
        emit receivedCurrentSec(sec);
    }
    else if(rx_endoffile.indexIn(line) > -1){
        qDebug("MplayerProcess::parseline:detected end of file");
        received_end_of_file = true;
    }
    else if(rx_winresolution.indexIn(line) > -1){
        int w = rx_winresolution.cap(4).toInt();
        int h = rx_winresolution.cap(5).toInt();

        emit receivedWindowResolution(w,h);
    }
    else if(rx_paused.indexIn(line) > -1){
        emit receivedPause();
    }
    else if(rx.indexIn(line) > -1){
        tag = rx.cap(1);
        value = rx.cap(2);
        //qDebug()<<"tag="<<tag<<"value="<<value;
        if(tag == "ID_LENGTH"){
            emit getLength(value.toDouble());
        }
        /*else if(tag == "ID_VIDEO_WIDTH"){
                dcpinfo.video_width = value.toInt();
            }
            else if(tag == "ID_VIDEO_HEIGHT"){
                dcpinfo.video_height = value.toInt();
            }
            else if(tag == "ID_VIDEO_ASPECT"){
                dcpinfo.video_aspect = (double)dcpinfo.video_width/dcpinfo.video_height;
            }*/
    }
}

int MyplayerProcess::canReadLine(const QByteArray & ba,int from){
    int pos1 = ba.indexOf('\n',from);
    int pos2 = ba.indexOf('\r', from);

    //qDebug("MyProcess::canReadLine: pos2: %d", pos2);

    if ( (pos1 == -1) && (pos2 == -1) ) return -1;

    int pos = pos1;
    if ( (pos1 != -1) && (pos2 != -1) ) {
        /*
            if (pos2 == (pos1+1)) pos = pos2; // \r\n
            else
            */
        if (pos1 < pos2) pos = pos1; else pos = pos2;
    } else {
        if (pos1 == -1) pos = pos2;
        else
            if (pos2 == -1) pos = pos1;
    }

    return pos;
}
