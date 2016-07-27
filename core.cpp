#include "core.h"
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>
#include <cmath>
#include <QDebug>

#include "mplayerwindow.h"
#include "desktopinfo.h"
#include "dcpdata.h"
#include "socketconnect.h"
#include "playlist.h"
#include "base64.h"
#include "global.h"

using namespace Global;

extern QString g_appid;

Core::Core(MplayerWindow *mpw,QWidget *parent)
    : QObject(parent)
{
    qRegisterMetaType<Core::State>("Core::State");

    mplayerwindow = mpw;

    _state = READY;

    proc = new MyplayerProcess();

    socket = new SocketConnect();

    playlist =  new PlayList();

    testnum = 0;

    nownum = 0;

    //dcpinfo = new DcpData();
    is_selectspl = false;

    connect(proc,SIGNAL(processExited()),
            mplayerwindow->videoLayer(),SLOT(playingStopped()));
    connect(proc,SIGNAL(error(QProcess::ProcessError)),
            mplayerwindow->videoLayer(),SLOT(playingStopped()));
    connect(proc,SIGNAL(processExited()),
            mplayerwindow,SLOT(playingStopped()));
    connect(proc,SIGNAL(error(QProcess::ProcessError)),
            mplayerwindow,SLOT(playingStopped()));

    connect(proc,SIGNAL(processExited()),
            this,SLOT(processFinished()),Qt::QueuedConnection);

    connect(this,SIGNAL(aboutToStartPlaying()),
            mplayerwindow->videoLayer(),SLOT(playingStarted()));
    connect(this,SIGNAL(aboutToStartPlaying()),
            mplayerwindow,SLOT(playingStarted()));
    connect(proc,SIGNAL(getLength(double)),
            this,SLOT(changeTotalTime(double)));
    connect(proc,SIGNAL(receivedCurrentSec(double)),
            this,SLOT(changeCurrentSec(double)));
    connect(proc,SIGNAL(receivedWindowResolution(int,int)),
            this,SLOT(gotWindowResolution(int,int)));

    connect(proc,SIGNAL(receivedPause()),
            this,SLOT(changePause()));
    connect(proc,SIGNAL(receivedEndOfFile()),
            this,SLOT(fileReachedEnd()),Qt::QueuedConnection);

    connect(socket,&SocketConnect::createspl,
            this,&Core::createspl);
    connect(socket,&SocketConnect::selectspl,
            this,&Core::selectspl);
    connect(socket,&SocketConnect::playspl,
            this,&Core::playspl);
    connect(socket,&SocketConnect::stopspl,
            this,&Core::stopspl);
    connect(socket,&SocketConnect::get_status,
            this,&Core::getsocketstatus);
    connect(socket,&SocketConnect::getprikeyinfo,
            this,&Core::getprikey);
    connect(socket,&SocketConnect::setprikeyinfo,
            this,&Core::setprikey);
    connect(socket,&SocketConnect::getservertime,
            this,&Core::gettime);
    connect(socket,&SocketConnect::setserverdatetime,
            this,&Core::setserverdatetime);

    mset.reset();
    dcpinfo.reset();
    mplayerwindow->videoLayer()->setRepaintBackground(false);
    mplayerwindow->setMonitorAspect((double)16/9);
}

Core::~Core(){
    if(proc->isRunning())stopMplayer();
    proc->terminate();
    delete proc;
    QFile f;
    f.remove("platforms/test.enc");
    //delete dcpinfo;
}

void Core::setState(State s){
    _state = s;
}

void Core::openDcp(QString video, QString audio, QString key_in){
    qDebug("Core::openDcp:status %d",_state);
    //mset.reset();
    mplayerwindow->hideLogo();
    if(proc->isRunning()){
        qDebug()<<"restart";
        stopMplayer();
        proc->waitForFinished();
    }
    proc->setCommonOptions();
    proc->setVolumeOptions(mset.volume);
    proc->setShowscreenOptions(QString::number((qint64) mplayerwindow->videoLayer()->winId()));
    if(key_in != ""){
        proc->setKeyOptions(key_in);
    }
    proc->setDcpOptions(video,audio);
    emit aboutToStartPlaying();
    proc->start();
    if(proc->isRunning()){
        setState(PLAYING);
        emit stateChanged(PLAYING);
    }
}

void Core::playTest(int num){
    qDebug("Core::playTest:%d",num);
    dcpinfo.parse_nfs_dcp(playlist->pl.at(num).dcpurl(),playlist->pl.at(num).kdmurl());

    qDebug()<<"Media is "<<playlist->pl.at(num).dcpurl();
    if(proc->isRunning()){
        qDebug()<<"restart";
        stopMplayer();
        proc->waitForFinished();
    }
    //mplayerwindow->hideLogo();
    proc->setCommonOptions();
    proc->setChannel(mset.audio_use_channels);

    if(mset.stereo_mode != 0){
        switch(mset.stereo_mode){
        case MediaSettings::Left:proc->addAF("channels","2:2:0:1:0:0");break;
        case MediaSettings::Right:proc->addAF("channels","2:2:1:0:1:1");break;
        case MediaSettings::Mono:proc->addAF("pan","1:0.5:0.5");break;
        case MediaSettings::Reverse:proc->addAF("channels","2:2:0:1:1:0");break;
        }
    }
    proc->setVolumeOptions(mset.volume);
    proc->setShowscreenOptions(QString::number((qint64) mplayerwindow->videoLayer()->winId()));
    QString key_ = dcpinfo.key;
    if(key_ != ""){
        proc->setKeyOptions(key_);
    }
    if(dcpinfo.video_url.contains(":")){
        proc->setDcpOptions(dcpinfo.video_url,dcpinfo.audio_url);
    }else{
        QString video = playlist->pl.at(num).dcpurl() + "/" + dcpinfo.video_url;
        video = "nfs://" + video.remove(":");
        QString audio = playlist->pl.at(num).dcpurl() + "/" + dcpinfo.audio_url;
        audio = "nfs://" + audio.remove(":");
        proc->setDcpOptions(video,audio);
    }
    emit aboutToStartPlaying();
    proc->start();
    setState(PLAYING);
    emit stateChanged(PLAYING);
    emit gotoplayerpage(0);
}

void Core::restartPlay(){
    qDebug("Core::restartPlay");
    if(proc->isRunning()){
        qDebug()<<"restart";
        stopMplayer();
        proc->waitForFinished();
    }
    mplayerwindow->hideLogo();
    proc->setCommonOptions();
    proc->setChannel(mset.audio_use_channels);
    if(dcpinfo.current_sec != 0){
        proc->setStartTime(dcpinfo.current_sec);
    }
    if(mset.stereo_mode != 0){
        switch(mset.stereo_mode){
        case MediaSettings::Left:proc->addAF("channels","2:2:0:1:0:0");break;
        case MediaSettings::Right:proc->addAF("channels","2:2:1:0:1:1");break;
        case MediaSettings::Mono:proc->addAF("pan","1:0.5:0.5");break;
        case MediaSettings::Reverse:proc->addAF("channels","2:2:0:1:1:0");break;
        }
    }
    proc->setVolumeOptions(mset.volume);
    proc->setShowscreenOptions(QString::number((qint64) mplayerwindow->videoLayer()->winId()));
    QString key_ = dcpinfo.key;
    if(key_ != ""){
        proc->setKeyOptions(key_);
    }
    if(dcpinfo.video_url.contains(":")){
        proc->setDcpOptions(dcpinfo.video_url,dcpinfo.audio_url);
    }else{
        QString video = playlist->pl.at(0).dcpurl() + "/" + dcpinfo.video_url;
        video = "nfs://" + video.remove(":");
        QString audio = playlist->pl.at(0).dcpurl() + "/" + dcpinfo.audio_url;
        audio = "nfs://" + audio.remove(":");
        proc->setDcpOptions(video,audio);
    }
    emit aboutToStartPlaying();
    proc->start();
    setState(PLAYING);
    emit stateChanged(PLAYING);
    emit gotoplayerpage(0);
}

QString Core::pausing_prefix(){
    return "pausing_keep_force";
}

void Core::stopMplayer(){
    if(!proc->isRunning()){
        return;
    }
    proc->quit();
    if(!proc->waitForFinished(100)){
        proc->kill();
    }
}

void Core::play(){
    if((proc->isRunning())  && (_state == PAUSED)){
        proc->setPause(false);
        _state = PLAYING;
        emit stateChanged(PLAYING);
    }
    else if((proc->isRunning())  && (_state == PLAYING)){
        //nothing to do
    }

}

void Core::pause(){
    if(proc->isRunning()){
        qDebug()<<"state="<<state();
        if(_state == PAUSED) {
            proc->setPause(false);
            _state = PLAYING;
            emit stateChanged(PLAYING);
        }
        else {
            proc->setPause(true);
            _state = PAUSED;
            emit stateChanged(PAUSED);
        }
    }
}

void Core::play_or_pause(){
    if(proc->isRunning()){
        pause();
    }else{
        play();
    }
}

void Core::stop(){
    qDebug("Core::Stop");
    if(state() == STOPPED||state() == PLAYING){
        dcpinfo.current_sec = 0;
        emit showTime(dcpinfo.current_sec);
        emit positionChanged(0);
    }

    stopMplayer();
    setState(READY);
    emit mediaStoppedByUser();
}

void Core::seek(int secs){
    qDebug("Core::seek: %d",secs);
    if((proc->isRunning()) && (secs)!=0){
        seek_cmd(secs,0);
    }
}

void Core::sforward(){
    qDebug("Core::sforward");
    seek(10);
}

void Core::srewind(){
    qDebug("Core::srewind");
    seek(-10);
}

void Core::forward(){
    qDebug("Core::forward");
    seek(60);
}

void Core::rewind(){
    qDebug("Core::rewind");
    seek(-60);
}

void Core::fastforward(){
    qDebug("Core::fastforward");
    seek(600);
}

void Core::fastrewind(){
    qDebug("Core::fastrewind");
    seek(-600);
}

void Core::processFinished(){
    qDebug("Core::processFinished");
    int exit_code = proc->exitCode();
    qDebug("Core::processFinished:play has finished with exitcode %d",exit_code);
    if(exit_code != 0){
        setState(STOPPED);
        emit stateChanged(STOPPED);
        emit mplayerFinishedWithError(exit_code);
    }
    //emit stateChanged(READY);
}

void Core::changePause(){
    setState(PAUSED);
}

void Core::goToPosition(int value){
    goToPos((double)value/10);
}

void Core::goToPos(double perc){
    seek_cmd(perc,1);
}

void Core::changeTotalTime(double secs){
    qDebug()<<"change time";
    dcpinfo.duration = secs;
    emit newDuration(secs);
}

void Core::seek_cmd(double secs, int mode){
    proc->seek(secs,mode,true);
}

void Core::changeCurrentSec(double sec){
    if(state() != PLAYING){
        //setState(PLAYING);
        //emit stateChanged(PLAYING);
    }
    dcpinfo.current_sec = sec;
    emit showTime(dcpinfo.current_sec);

    static int last_second = 0;
    if(floor(sec) == last_second)return;
    last_second = (int)floor(sec);

    int value = 0;
    value = ((int) (sec*1000)/(int)dcpinfo.duration);
    emit positionChanged(value);
}

void Core::changeZoom(double p){
    qDebug("Core::changeZoom: %f",p);
    if(p < 0.5) p = 0.5;
    mset.zoom_factor = p;
    mplayerwindow->setZoom(p);
}

void Core::resetZoom(){
    changeZoom(1.0);
}

void Core::autoZoom(){
    double video_aspect = mset.aspectToNum((MediaSettings::Aspect)mset.aspect_ratio_id);

    if(video_aspect <= 0){
        QSize w = mplayerwindow->videoLayer()->size();
        video_aspect = (double)w.width()/w.height();
    }

    double screen_aspect = DesktopInfo::desktop_aspectRatio(mplayerwindow);
    double zoom_factor;

    if(video_aspect > screen_aspect)
        zoom_factor = video_aspect / screen_aspect;
    else
        zoom_factor = screen_aspect / video_aspect;

    changeZoom(zoom_factor);
}

void Core::autoZoomFromLetterbox(double aspect){
    qDebug("Core::autoZoomFromLetterbox:%f",aspect);

    QSize desktop = DesktopInfo::desktop_size(mplayerwindow);

    double video_aspect = mset.aspectToNum((MediaSettings::Aspect)mset.aspect_ratio_id);

    if(video_aspect <= 0){
        QSize w = mplayerwindow->videoLayer()->size();
        video_aspect = (double)w.width()/w.height();
    }
    QSize video;
    video.setHeight(desktop.height());
    video.setWidth((int)(video.height()*video_aspect));
    if(video.width() > desktop.width()){
        video.setWidth(desktop.width());
        video.setHeight((int)(video.width()/video_aspect));
    }

    QSize actual_video;
    actual_video.setWidth(video.width());
    actual_video.setHeight((int)(video.width()/aspect));

    double zoom_factor = (double)desktop.height()/actual_video.height();
    changeZoom(zoom_factor);
}

void Core::autoZoomFor169(){
    autoZoomFromLetterbox((double)16/9);
}

void Core::autoZoomFor235(){
    autoZoomFromLetterbox(2.35);
}

void Core::incZoom(){
    qDebug("Core::incZoom");
    changeZoom(mset.zoom_factor + ZOOM_STEP);
}

void Core::decZoom(){
    qDebug("Core::decZoom");
    changeZoom(mset.zoom_factor - ZOOM_STEP);
}

void Core::mute(bool b){
    qDebug()<<"mute"<<b;
    proc->setPausingPrefix(pausing_prefix());
    proc->mute(b);
    mset.mute = b;
    emit signalmute(b);
}

void Core::setVolume(int volume, bool force){
    int current_volume = mset.volume;
    if((volume == current_volume) && (!force))return;

    current_volume = volume;
    if(current_volume > 100) current_volume = 100;
    if(current_volume < 0) current_volume = 0;

    proc->setVolume(current_volume);

    mset.volume = current_volume;

    emit volumeChanged(current_volume);
}

void Core::incVolume(){
    qDebug("Core::incVolume");
    setVolume(mset.volume + 4);
}

void Core::decVolume(){
    qDebug("Core::decVolume");
    setVolume(mset.volume - 4);
}

void Core::setAudioDelay(int delay){
    qDebug("Core::setAudioDelay %d",delay);
    mset.audio_delay = delay;
    proc->setPausingPrefix(pausing_prefix());
    proc->setAudioDelay(delay);
}

void Core::incAudioDelay(){
    qDebug("Core::incAudioDelay");
    setAudioDelay(mset.audio_delay + 100);
}

void Core::decAudioDelay(){
    qDebug("Core::decAudioDelay");
    setAudioDelay(mset.audio_delay - 100);
}

void Core::gotWindowResolution(int w, int h){
    emit needResize(w,h);

    mset.win_width = w;
    mset.win_height = h;
    mplayerwindow->setResolution(w,h);
    qDebug("gotwindowResolution aspect=%f",mset.win_aspect());
    mplayerwindow->setAspect(mset.win_aspect());

}

void Core::changeAspectRatio(int ID){
    qDebug("Core::changeAspectRatio:%d",ID);

    mset.aspect_ratio_id = ID;

    double asp = mset.aspectToNum((MediaSettings::Aspect)ID);

    mplayerwindow->setAspect(asp);

    proc->setAspect(asp);
}

void Core::setAudioChannels(int channels){
    qDebug("Core::setAudioChannels:%d",channels);
    if(channels != mset.audio_use_channels){
        mset.audio_use_channels = channels;
        restartPlay();
    }
}

void Core::setStereoMode(int mode){
    qDebug("Core::setStereoMode %d",mode);
    if(mode != mset.stereo_mode){
        mset.stereo_mode = mode;
        restartPlay();
    }
}

void Core::fileReachedEnd(){
    mset.reset();
    if (nownum < (testnum-1)&(testnum != 0))
    {
        nownum ++;
        playTest(nownum);
    }else{
    emit mediaFinished();
    }
}

void Core::createspl(){
    qDebug("Core::createspl");
    dcpinfo.reset();
    mset.reset();
    is_selectspl = false;
    playlist = socket->playlist;
    dcpinfo.dcp_title = playlist->splname;
    socket->make_response("",0);
    socket->send_response();
}

void Core::selectspl(){
    qDebug("Core::select_spl");
    QString dcp_path = playlist->pl.at(0).dcpurl();
    QString kdm_path = playlist->pl.at(0).kdmurl();
    int ret = 0;
    //qDebug()<<prikey;
    if(ret = dcpinfo.parse_nfs_dcp(dcp_path,kdm_path) != 0){
        qDebug("Core::selectspl:error! ret = %d",ret);
        socket->make_response("Can not select this spl.",-1);
        socket->send_response();
        is_selectspl = false;
    }
    else{
        qDebug()<<dcpinfo.kdm_info.nv_after<<dcpinfo.kdm_info.nv_before;
        is_selectspl = true;
        socket->make_response("",0);
        socket->send_response();
    }
}

void Core::playspl(){
    qDebug("Core::play_spl");
    if(!is_selectspl){
        socket->make_response("Can not play this spl.",-1);
        socket->send_response();
    }else{
        mplayerwindow->hideLogo();
        if(proc->isRunning()){
            qDebug()<<"restart";
            stopMplayer();
            proc->waitForFinished();
        }
        //qDebug()<<"Core::playspl:"<<key_in;
        proc->setCommonOptions();
        proc->setVolumeOptions(mset.volume);
        proc->setShowscreenOptions(QString::number((qint64) mplayerwindow->videoLayer()->winId()));
        QString key_ = dcpinfo.key;
        if(key_ != ""){
            proc->setKeyOptions(key_);
        }
        QString video = playlist->pl.at(0).dcpurl() + "/" + dcpinfo.video_url;
        video = "nfs://" + video.remove(":");
        QString audio = playlist->pl.at(0).dcpurl() + "/" + dcpinfo.audio_url;
        audio = "nfs://" + audio.remove(":");
        proc->setDcpOptions(video,audio);
        emit aboutToStartPlaying();
        proc->start();
        setState(PLAYING);
        emit stateChanged(PLAYING);
        socket->make_response("",0);
        socket->send_response();
        emit gotoplayerpage(0);
    }
}

void Core::stopspl(){
    qDebug("Core::stopSpl");
    playlist->clear();
    stop();
    socket->make_response("",0);
    socket->send_response();
}

void Core::getsocketstatus(){
    qDebug("Core::getsocketstatus");
    socket->response_getstatus(_state,dcpinfo.cpl_uuid,dcpinfo.current_sec);
}

void Core::getprikey(){
    qDebug("Core::getprikey");
    socket->responseGetprikeyInfo(g_appid);
}

void Core::setprikey(const char* prikeyValue){
    qDebug("Core::setprikey");
    unsigned int out_size = 0,alloc_size = 0;
    void *p_data = base64_dec(prikeyValue, &out_size, &alloc_size);
    if( p_data == 0 || out_size < 256)
    {
        if( p_data)
        {
            free(p_data);
        }
    }
    FILE *fs = fopen("platforms/test.enc","wb");
    fwrite(p_data,1696,1,fs);
    fclose(fs);
    socket->make_response("",0);
    socket->send_response();
}

void Core::gettime(){
    qDebug("Core::gettime");
    QDateTime time = QDateTime::currentDateTime();
    QString str = time.toString("yyyy-MM-ddThh:mm:ss+08:00");
    socket->responseGetserverTime(str);
}

void Core::setserverdatetime(QString time){
    qDebug("Core::setservertime");
}
