#ifndef CORE_H
#define CORE_H

#include <QObject>
#include <QProcess>
#include "myplayerprocess.h"
#include "dcpdata.h"
#include "mediasettings.h"

class MplayerWindow;
class DcpData;
class SocketConnect;
class PlayList;

class Core : public QObject
{
    Q_OBJECT
public:
    enum State{READY = 0,PLAYING = 1,PAUSED = 2,STOPPED = 3};
    Core(MplayerWindow *mpw, QWidget *parent = 0);
    ~Core();

    State state(){return _state;};

    MediaSettings mset;
    DcpData dcpinfo;
    PlayList *playlist;

    void openDcp(QString video,QString audio,QString key_in = 0);
    void playTest(int num);
    int testnum;
    int nownum;
    //void setDcp(DcpData *data){dcpinfo = data;};

protected:
    void stopMplayer();
    void restartPlay();
    void seek_cmd(double secs,int mode);
    void setState(State s);
    QString pausing_prefix();

signals:
    void mediaStoppedByUser();
    void aboutToStartPlaying();
    void positionChanged(int);
    void newDuration(double);
    void showTime(double);
    void volumeChanged(int);
    void needResize(int w,int h);
    void stateChanged(Core::State state);
    void signalmute(bool b);
    void mediaFinished();
    void mplayerFinishedWithError(int);
    void gotoplayerpage(int);

public slots:
    void play();
    void play_or_pause();
    void pause();
    void stop();
    void goToPosition(int value);
    void goToPos(double perc);
    void mute(bool);
    void setVolume(int volume,bool force = false);
    void seek(int secs);
    void sforward();         //+10 seconds
    void srewind();          //-10 Seconds
    void forward();          //+1 minute
    void rewind();           //-1 minute
    void fastforward();  //+10 minutes
    void fastrewind();   //-10 minutes
    void changeAspectRatio(int);
    void changeZoom(double);
    void autoZoomFromLetterbox(double aspect);
    void resetZoom();
    void autoZoom();
    void autoZoomFor169();
    void autoZoomFor235();
    void incZoom();
    void decZoom();
    void setAudioChannels(int);
    void setStereoMode(int mode);
    void incVolume();
    void decVolume();
    void setAudioDelay(int delay);
    void decAudioDelay();
    void incAudioDelay();

protected slots:
    void changeCurrentSec(double sec);
    void changeTotalTime(double secs);
    void gotWindowResolution(int w,int h);
    void changePause();
    void fileReachedEnd();
    void processFinished();
    void createspl();
    void selectspl();
    void playspl();
    void stopspl();
    void getprikey();
    void setprikey(const char* prikeyValue);
    void gettime();
    void getsocketstatus();
    void setserverdatetime(QString time);

protected:
    MyplayerProcess *proc;
    MplayerWindow *mplayerwindow;
    SocketConnect *socket;


private:
    State _state;
    const char * prikey;
    bool is_selectspl;
    //DcpData *dcpinfo;
};

#endif // CORE_H
