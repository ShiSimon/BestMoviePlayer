#ifndef MYPLAYERPROCESS_H
#define MYPLAYERPROCESS_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QVariant>

class MyplayerProcess : public QProcess
{
    Q_OBJECT
public:
    MyplayerProcess(QObject *parent = 0);
    bool start();
    bool isRunning();
    void resetDcp();
    void setCommonOptions();
    void setChannel(int num);
    void addAF(const QString & filter_name,const QVariant & value = QVariant());
    void setStartTime(double sec);
    void setVolumeOptions(int v);
    void setShowscreenOptions(QString winID);
    void setKeyOptions(QString keyin);
    void setDcpOptions(QString video_name,QString audio_name);
    void setPausingPrefix(const QString & prefix){pausing_prefix = prefix;};
    //slave command
    void setPause(bool b);
    void seek(double secs,int mode,bool precise);
    void mute(bool b);
    void setVolume(int v);
    void setAspect(double asp);
    void setAudioDelay(double delay);
    void quit();

signals:
    void receivedCurrentSec(double sec);
    void getLength(double secs);
    void lineAvaiable(QByteArray ba);
    void receivedPause();
    void processExited();
    void receivedEndOfFile();
    void receivedWindowResolution(int,int);

protected slots:
    void readStdOut();
    void procFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void parseLine(QByteArray line);

protected:
    int canReadLine(const QByteArray & ba,int from=0);

protected:
    QString mplayer_bin;
    QStringList arg;
    QString pausing_prefix;
    QByteArray remaining_output;

private:
    bool received_end_of_file;

};

#endif // MYPLAYERPROCESS_H
