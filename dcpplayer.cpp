#include "dcpplayer.h"
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QHBoxLayout>
#include <QCursor>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QRegExp>
#include <QDebug>
#include <QToolBar>
#include <QMenuBar>
#include <cmath>
#include <QFileDialog>
#include <QTimer>
#include <QPixmap>
#include <QTabWidget>
#include <QTabBar>
#include <QToolBar>
#include <QMouseEvent>
#include <QFont>
#include <QStyle>
#include <QStyleFactory>
#include <QKeyEvent>
#include <QFile>

#include "mplayerwindow.h"
#include "autohidewidget.h"
#include "myaction.h"
#include "timeslider.h"
#include "mywidgetaction.h"
#include "core.h"
#include "images.h"
#include "helper.h"
#include "webpage.h"
#include "myactiongroup.h"
#include "about.h"
#include "global.h"
#include "playlist.h"

using namespace Global;

DcpPlayer::DcpPlayer(QWidget *parent,Qt::WindowFlags flags)
    : QMainWindow(parent,flags)
    ,was_maximized(false)
    ,popup(0)
{
    setWindowTitle("BestvMovie");
    showMaximized();
    //Create objects:
    createPanel();
    setCentralWidget(tabwidget);

    createMplayerWindow();
    createCore();
    createActions();
    createMenus();

    setAcceptDrops(true);
    createHideBar();
    createToolBar();
    mplayerwindow->videoLayer()->setRepaintBackground(false);
    wp = new WebPage(this);
    wp->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    wp->setMinimumSize(QSize(1,1));
    wp->setFocusPolicy(Qt::StrongFocus);
    tabwidget->addTab(wp,tr("片库"));
    tabwidget->setFocusPolicy(Qt::NoFocus);
    retranslateStrings();
    panel->setFocus();
    tabwidget->setCurrentIndex(1);
    control_tools->hide();
    connect(tabwidget,SIGNAL(currentChanged(int)),this,SLOT(updateTabwidget(int)));
    QApplication::setStyle(QStyleFactory::create("Fushion"));
}

DcpPlayer::~DcpPlayer()
{
    delete core; //delete before mplayerwindow
    delete wp;   //delete webpage
}

void DcpPlayer::createCore(){
    core = new Core(mplayerwindow,this);
    connect(core,SIGNAL(mediaStoppedByUser()),this,SLOT(disableActionsOnStop()));
    connect(core,SIGNAL(showTime(double)),this,SLOT(gotCurrentTime(double)));

    connect(core,SIGNAL(needResize(int,int)),this,SLOT(resizeWindow(int,int)));
    connect(core,SIGNAL(needResize(int,int)),this,SLOT(centerWindow()));

    connect(core,SIGNAL(stateChanged(Core::State)),this,SLOT(updateWidget(Core::State)));
    connect(core,SIGNAL(signalmute(bool)),this,SLOT(setMute(bool)));

    connect(core,SIGNAL(mediaFinished()),this,SLOT(disableActionsOnStop()));
    connect(core,SIGNAL(mplayerFinishedWithError(int)),
            this,SLOT(showExitCodeFromMplayer(int)));

    connect(core,SIGNAL(mediaStoppedByUser()),
            mplayerwindow,SLOT(showLogo()));

    connect(core,SIGNAL(gotoplayerpage(int)),this,SLOT(updateTabwidget(int)));
}

void DcpPlayer::createMenus(){
    //MENU
    openMenu = menuBar()->addMenu(tr("打开"));
    playMenu = menuBar()->addMenu(tr("控制"));
    videoMenu = menuBar()->addMenu(tr("视频"));
    audioMenu = menuBar()->addMenu(tr("音频"));
    helpMenu = menuBar()->addMenu(tr("帮助"));
    menuBar()->show();

    //Open Menu
    openMenu->addAction(openDcpAct);
    openMenu->addAction(testDcp);
    openMenu->addAction(exitAct);

    //PLAY MENU
    playMenu->addAction(playOrPauseAct);
    playMenu->addAction(stopAct);
    playMenu->addSeparator();
    playMenu->addAction(forward1Act);
    playMenu->addAction(forward2Act);
    playMenu->addAction(forward3Act);
    playMenu->addAction(rewind1Act);
    playMenu->addAction(rewind2Act);
    playMenu->addAction(rewind3Act);

    //Video Menu
    aspectMenu =  new QMenu(this);
    aspectMenu->menuAction()->setObjectName("aspect_menu");
    aspectMenu->menuAction()->setText("视频比例");
    aspectMenu->addActions(aspectGroup->actions());
    videoMenu->addMenu(aspectMenu);

    videosize_menu = new QMenu(this);
    videosize_menu->menuAction()->setObjectName("videosize_menu");
    videosize_menu->addActions(sizeGroup->actions());
    //videoMenu->addMenu(videosize_menu);

    zoom_menu = new QMenu(this);
    zoom_menu->menuAction()->setObjectName("zoom_menu");
    zoom_menu->addAction(resetZoomAct);
    zoom_menu->addSeparator();
    zoom_menu->addAction(autoZoomAct);
    zoom_menu->addAction(autoZoom169Act);
    zoom_menu->addAction(autoZoom235Act);
    zoom_menu->addSeparator();
    zoom_menu->addAction(decZoomAct);
    zoom_menu->addAction(incZoomAct);
    zoom_menu->addSeparator();
    zoom_menu->addAction(moveLeftAct);
    zoom_menu->addAction(moveRightAct);
    zoom_menu->addAction(moveUpAct);
    zoom_menu->addAction(moveDownAct);
    videoMenu->addMenu(zoom_menu);

    videoMenu->addAction(fullScreenAct);

    //Audio Menu
    audiochannels_menu = new QMenu(this);
    audiochannels_menu->menuAction()->setObjectName("audiochannels_menu");
    audiochannels_menu->addActions(channelsGroup->actions());
    audioMenu->addMenu(audiochannels_menu);
    stereomode_menu = new QMenu(this);
    stereomode_menu->menuAction()->setObjectName("stereochannels_menu");
    stereomode_menu->addActions(stereoGroup->actions());
    audioMenu->addMenu(stereomode_menu);
    audioMenu->addSeparator();
    audioMenu->addAction(muteAction);
    audioMenu->addSeparator();
    audioMenu->addAction(incVolumeAct);
    audioMenu->addAction(decVolumeAct);
    audioMenu->addSeparator();
    audioMenu->addAction(incAudioDelayAct);
    audioMenu->addAction(decAudioDelayAct);


    //HELP MENU
    helpMenu->addAction(aboutThisAct);

    if(!popup)
        popup = new QMenu(this);
    else
        popup->clear();

    popup->addMenu(openMenu);
    popup->addMenu(playMenu);
    popup->addMenu(videoMenu);
    popup->addMenu(audioMenu);
    popup->addMenu(helpMenu);

}

void DcpPlayer::createToolBar(){
    control_tools = new QToolBar(panel);
    control_tools->setObjectName("control_tools");
    control_tools->setLayoutDirection(Qt::LeftToRight);
    control_tools->setAllowedAreas(Qt::BottomToolBarArea);
    control_tools->setMovable(false);
    control_tools->addAction(playOrPauseAct);
    control_tools->addAction(stopAct);
    control_tools->addSeparator();
    control_tools->addAction(time_label_action);
    control_tools->addAction(timeslider_action);
    control_tools->addAction(fullScreenAct);
    control_tools->addAction(muteAction);
    control_tools->addAction(volumeslider_action);
    control_tools->setIconSize(QSize(40,40));
    addToolBar(Qt::BottomToolBarArea,control_tools);
    //control_tools->setAllowedAreas(Qt::BottomToolBarArea);
}

void DcpPlayer::createActions()
{
    playOrPauseAct = new MyAction(Qt::Key_Space,this,"play_or_pause");
    playOrPauseAct->addShortcut(QKeySequence("space"));
    playOrPauseAct->setText(tr("播放"));
    connect(playOrPauseAct,SIGNAL(triggered(bool)),core,SLOT(play_or_pause()));

    stopAct = new MyAction(Qt::Key_MediaStop,this,"stop");
    stopAct->setText(tr("停止"));
    connect(stopAct,SIGNAL(triggered(bool)),
            core,SLOT(stop()));

    fullScreenAct = new MyAction(QKeySequence("F"),this,"fullscreen");
    fullScreenAct->addShortcut(QKeySequence("F"));
    fullScreenAct->setCheckable(true);
    fullScreenAct->setText(tr("全屏"));

    connect(fullScreenAct,SIGNAL(triggered(bool)),this,SLOT(toggleFullscreen(bool)));

    exitFullscreenAct = new MyAction(Qt::Key_Escape,this,"exit_fullscreen");
    exitFullscreenAct->setText(tr("退出全屏"));
    connect(exitFullscreenAct,SIGNAL(triggered(bool)),this,SLOT(exitFullscreen()));

    timeslider_action = new TimeSliderAction(this);
    timeslider_action->setObjectName("timeslider_action");
    connect(timeslider_action,SIGNAL(posChanged(int)),core,SLOT(goToPosition(int)));
    connect(core,SIGNAL(positionChanged(int)),timeslider_action,SLOT(setPos(int)));
    //connect(timeslider_action,SIGNAL(wheelUp()),core,SLOT(wheelUp()));
    //connect(timeslider_action,SIGNAL(wheelDown()),core,SLOT(wheelDown()));
    connect(timeslider_action,SIGNAL(draggingPos(int)),this,SLOT(displayGotoTime(int)));
    connect(core,SIGNAL(newDuration(double)),timeslider_action,SLOT(setDuration(double)));

    time_label_action = new TimeLabelAction(TimeLabelAction::CurrentAndTotalTime,this);
    time_label_action->setObjectName("timelabel_action");

    connect(this,SIGNAL(timeChanged(double)),time_label_action,SLOT(setCurrentTime(double)));
    connect(core,SIGNAL(newDuration(double)),time_label_action,SLOT(setTotalTime(double)));

    muteAction = new MyAction(Qt::Key_M,this,"mute");
    muteAction->addShortcut(Qt::Key_VolumeMute);
    muteAction->setText(tr("静音"));
    muteAction->setCheckable(true);
    connect(muteAction,SIGNAL(triggered(bool)),
            core,SLOT(mute(bool)));

    volumeslider_action = new VolumeSliderAction(this);
    volumeslider_action->setObjectName("volumeslider_action");

    connect(volumeslider_action,SIGNAL(valueChanged(int)),
            core,SLOT(setVolume(int)));
    connect(core,SIGNAL(volumeChanged(int)),
            volumeslider_action,SLOT(setValue(int)));

    rewind1Act = new MyAction(Qt::Key_Left,this,"rewind1");
    rewind1Act->addShortcut(QKeySequence("Shift+Ctrl+B"));
    connect(rewind1Act,SIGNAL(triggered(bool)),
            core,SLOT(srewind()));

    rewind2Act = new MyAction(Qt::Key_Down,this,"rewind2");
    connect(rewind2Act,SIGNAL(triggered(bool)),
            core,SLOT(rewind()));

    rewind3Act = new MyAction(Qt::Key_PageDown,this,"rewind3");
    connect(rewind3Act,SIGNAL(triggered(bool)),
            core,SLOT(fastrewind()));

    forward1Act = new MyAction(Qt::Key_Left,this,"forward1");
    forward1Act->addShortcut(QKeySequence("Shift+Ctrl+F"));
    connect(forward1Act,SIGNAL(triggered(bool)),
            core,SLOT(sforward()));

    forward2Act = new MyAction(Qt::Key_Up,this,"forward2");
    connect(forward2Act,SIGNAL(triggered(bool)),
            core,SLOT(forward()));

    forward3Act = new MyAction(Qt::Key_PageUp,this,"forward3");
    connect(forward3Act,SIGNAL(triggered(bool)),
            core,SLOT(fastforward()));

    //Menu Action
    testDcp = new MyAction(QKeySequence("Ctrl+T"),this,"test");
    testDcp->setText(tr("测试DCP电影"));
    connect(testDcp,SIGNAL(triggered(bool)),this,SLOT(playtest()));

    exitAct = new MyAction(QKeySequence("Ctrl+X"),this,"close");
    connect(exitAct,SIGNAL(triggered(bool)),this,SLOT(closeWindow()));

    //video aspect
    aspectGroup = new MyActionGroup(this);
    aspectDetectAct = new MyActionGroupItem(this,aspectGroup,"aspect_detect",MediaSettings::AspectAuto);
    aspect11Act = new MyActionGroupItem(this,aspectGroup,"aspect_1:1",MediaSettings::Aspect11);
    aspect54Act = new MyActionGroupItem(this,aspectGroup,"aspect_5:4",MediaSettings::Aspect54);
    aspect43Act = new MyActionGroupItem(this,aspectGroup,"aspect_4:3",MediaSettings::Aspect43);
    aspect118Act = new MyActionGroupItem(this,aspectGroup,"aspect_11:8",MediaSettings::Aspect118);
    aspect1410Act = new MyActionGroupItem(this,aspectGroup,"aspect_14:10",MediaSettings::Aspect1410);
    aspect32Act = new MyActionGroupItem(this,aspectGroup,"aspect_3:2",MediaSettings::Aspect32);
    aspect149Act = new MyActionGroupItem(this,aspectGroup,"aspect_14:9",MediaSettings::Aspect149);
    aspect1610Act = new MyActionGroupItem(this,aspectGroup,"aspect_16:10",MediaSettings::Aspect1610);
    aspect169Act = new MyActionGroupItem(this,aspectGroup,"aspect_16:9",MediaSettings::Aspect169);
    aspect235Act = new MyActionGroupItem(this,aspectGroup,"aspect_23.5:1",MediaSettings::Aspect235);
    connect(aspectGroup,SIGNAL(activated(int)),
            core,SLOT(changeAspectRatio(int)));

    //Video size
    sizeGroup = new MyActionGroup(this);
    size50 = new MyActionGroupItem(this,sizeGroup,"5&0%","size_50",50);
    size75 = new MyActionGroupItem(this,sizeGroup,"7&5%","size_75",75);
    size100 = new MyActionGroupItem(this,sizeGroup,"&100%","size_100",100);
    size125 = new MyActionGroupItem(this,sizeGroup,"1&25%","size_125",125);
    size150 = new MyActionGroupItem(this,sizeGroup,"15&0%","size_150",150);
    size175 = new MyActionGroupItem(this,sizeGroup,"1&75%","size_175",175);
    size200 = new MyActionGroupItem(this,sizeGroup,"&200%","size_200",200);
    size300 = new MyActionGroupItem(this,sizeGroup,"&300%","size_300",300);
    size400 = new MyActionGroupItem(this,sizeGroup,"&400%","size_400",400);
    size100->setShortcut(Qt::CTRL|Qt::Key_1);
    size200->setShortcut(Qt::CTRL|Qt::Key_2);
    connect(sizeGroup,SIGNAL(activated(int)),this,SLOT(changeSizeFactor(int)));
    //Make all not checkable
    QList<QAction *>size_list = sizeGroup->actions();
    for(int n = 0;n < size_list.count();n++){
        size_list[n]->setCheckable(false);
    }

    //Video Zoom
    moveUpAct = new MyAction(Qt::ALT|Qt::Key_Up,this,"move_up");
    connect(moveUpAct,SIGNAL(triggered(bool)),mplayerwindow,SLOT(moveUp()));

    moveDownAct = new MyAction(Qt::ALT|Qt::Key_Down,this,"move_down");
    connect(moveDownAct,SIGNAL(triggered(bool)),mplayerwindow,SLOT(moveDown()));

    moveLeftAct = new MyAction(Qt::ALT|Qt::Key_Left,this,"move_left");
    connect(moveLeftAct,SIGNAL(triggered(bool)),mplayerwindow,SLOT(moveLeft()));

    moveRightAct = new MyAction(Qt::ALT|Qt::Key_Right,this,"move_right");
    connect(moveRightAct,SIGNAL(triggered(bool)),mplayerwindow,SLOT(moveRight()));

    incZoomAct = new MyAction(Qt::Key_E,this,"inc_zoom");
    connect(incZoomAct,SIGNAL(triggered(bool)),core,SLOT(incZoom()));

    decZoomAct = new MyAction(Qt::Key_W,this,"dec_zoom");
    connect(decZoomAct,SIGNAL(triggered(bool)),core,SLOT(decZoom()));

    resetZoomAct = new MyAction(Qt::SHIFT|Qt::Key_E,this,"reset_zoom");
    connect(resetZoomAct,SIGNAL(triggered(bool)),core,SLOT(resetZoom()));

    autoZoomAct = new MyAction(Qt::SHIFT|Qt::Key_W,this,"auto_zoom");
    connect(autoZoomAct,SIGNAL(triggered(bool)),core,SLOT(autoZoom()));

    autoZoom169Act = new MyAction(Qt::SHIFT|Qt::Key_A,this,"zoom_169");
    connect(autoZoom169Act,SIGNAL(triggered(bool)),core,SLOT(autoZoomFor169()));

    autoZoom235Act = new MyAction(Qt::SHIFT|Qt::Key_S,this,"zoom_235");
    connect(autoZoom235Act,SIGNAL(triggered(bool)),core,SLOT(autoZoomFor235()));

    //Audio Menu Actions

    //Audio Channels
    channelsGroup = new MyActionGroup(this);

    channelsStereoAct = new MyActionGroupItem(this,channelsGroup,"channels_stereo",MediaSettings::ChStereo);
    channelsSurroundAct = new MyActionGroupItem(this,channelsGroup,"channels_surround",MediaSettings::ChSurround);
    channelsFull51Act = new MyActionGroupItem(this,channelsGroup,"channels_full51",MediaSettings::ChFull51);
    channelsFull61Act = new MyActionGroupItem(this,channelsGroup,"channels_full61",MediaSettings::ChFull61);
    connect(channelsGroup,SIGNAL(activated(int)),
            core,SLOT(setAudioChannels(int)));

    //Audio stereo Menu
    stereoGroup = new MyActionGroup(this);
    stereoAct = new MyActionGroupItem(this,stereoGroup,"stereo",MediaSettings::Stereo);
    leftChannelAct = new MyActionGroupItem(this,stereoGroup,"left_channel",MediaSettings::Left);
    rightChannelAct = new MyActionGroupItem(this,stereoGroup,"right_channel",MediaSettings::Right);
    monoAct = new MyActionGroupItem(this,stereoGroup,"mono",MediaSettings::Mono);
    reverseAct = new MyActionGroupItem(this,stereoGroup,"reverse",MediaSettings::Reverse);
    connect(stereoGroup,SIGNAL(activated(int)),
            core,SLOT(setStereoMode(int)));

    decVolumeAct =new MyAction(this,"decrease_volume");
    decVolumeAct->setShortcut(Qt::Key_9);
    connect(decVolumeAct,SIGNAL(triggered(bool)),
            core,SLOT(decVolume()));
    incVolumeAct =new MyAction(this,"increase_volume");
    incVolumeAct->setShortcut(Qt::Key_0);
    connect(incVolumeAct,SIGNAL(triggered(bool)),
            core,SLOT(incVolume()));
    decAudioDelayAct = new MyAction(Qt::Key_Minus,this,"dec_audio_delay");
    connect(decAudioDelayAct,SIGNAL(triggered(bool)),
            core,SLOT(decAudioDelay()));
    incAudioDelayAct = new MyAction(Qt::Key_Plus,this,"inc_audio_delay");
    connect(incAudioDelayAct,SIGNAL(triggered(bool)),
            core,SLOT(incAudioDelay()));

    openDcpAct = new MyAction(QKeySequence("Ctrl+F"),this,"open_file");
    openDcpAct->setText(tr("打开DCP电影"));
    connect(openDcpAct,SIGNAL(triggered(bool)),
            this,SLOT(openDcp()));

    aboutThisAct = new MyAction(this,"about_DcpPlayer");
    aboutThisAct->setText(tr("关于"));
    connect(aboutThisAct,SIGNAL(triggered(bool)),this,SLOT(helpAbout()));

    showContextMenuAct = new MyAction(this,"show_context_menu");
    connect(showContextMenuAct,SIGNAL(triggered(bool)),
            this,SLOT(showPopupMenu()));
}

void DcpPlayer::createHideBar(){
    hide_widget = new AutoHideWidget(mplayerwindow);
    hide_widget->setAutoHide(true);
    QToolBar *iw = new QToolBar(hide_widget);
    iw->setObjectName("hide_control");
    iw->addAction(playOrPauseAct);
    iw->addAction(stopAct);
    iw->addSeparator();
    iw->addAction(time_label_action);
    iw->addAction(timeslider_action);
    iw->addAction(fullScreenAct);
    iw->addAction(muteAction);
    iw->addAction(volumeslider_action);
    iw->setIconSize(QSize(40,40));
    hide_widget->setInternalWidget(iw);
    //hide_widget->adjustSize();
    hide_widget->hide();
    //hide_widget->deactivate();
    //QTimer::singleShot(100,hide_widget,SLOT(activate()));
}

void DcpPlayer::updateWidget(Core::State states){
    bool b;
    qDebug()<<"DcpPlayer::updatewidget"<<states;
    b = (states == Core::PLAYING) | (states == Core::PAUSED);
    if(b){
        if(states == Core::PLAYING){
            playOrPauseAct->setIcon(Images::icon("pause"));
            playOrPauseAct->setText(tr("暂停"));
        }
        else{
            playOrPauseAct->setIcon(Images::icon("play"));
            playOrPauseAct->setText(tr("播放"));
        }
    }
    else {
        playOrPauseAct->setIcon(Images::icon("play"));
        playOrPauseAct->setText(tr("播放"));
    }
    rewind1Act->setEnabled(b);
    rewind2Act->setEnabled(b);
    rewind3Act->setEnabled(b);
    forward1Act->setEnabled(b);
    forward2Act->setEnabled(b);
    forward3Act->setEnabled(b);
    decVolumeAct->setEnabled(b);
    incVolumeAct->setEnabled(b);
    incAudioDelayAct->setEnabled(b);
    decAudioDelayAct->setEnabled(b);
    fullScreenAct->setEnabled(b);
    muteAction->setEnabled(b);
    timeslider_action->setEnabled(b);
    volumeslider_action->setEnabled(b);
    aspectGroup->setActionsEnabled(b);
    aspectGroup->setChecked(core->mset.aspect_ratio_id);

    //Moving and Zoom
    moveUpAct->setEnabled(b);
    moveDownAct->setEnabled(b);
    moveLeftAct->setEnabled(b);
    moveRightAct->setEnabled(b);
    incZoomAct->setEnabled(b);
    decZoomAct->setEnabled(b);
    resetZoomAct->setEnabled(b);
    autoZoomAct->setEnabled(b);
    autoZoom169Act->setEnabled(b);
    autoZoom235Act->setEnabled(b);

    sizeGroup->setActionsEnabled(b);
    sizeGroup->setChecked(core->mset.factor);
    channelsGroup->setActionsEnabled(b);
    channelsGroup->setChecked(core->mset.audio_use_channels);
    stereoGroup->setActionsEnabled(b);
    stereoGroup->setChecked(core->mset.stereo_mode);
}

void DcpPlayer::openDcp(){
    qDebug("DcpPlayer::opendcp");
    QString s = QFileDialog::getExistingDirectory(this);
    if(s == "/"||s == "")
    {return;}
    openDcp(s);
}

void DcpPlayer::openDcp(QString file){
    qDebug()<<"DcpPlayer::opendcp:file="<<file;
    core->dcpinfo.reset();
    if(core->dcpinfo.is_Dcp_Dir(file)){
        core->dcpinfo.parseLocalDir();
    }
    if(core->dcpinfo.isenc){
        qDebug()<<"DcpPlayer::opendcp:get kdm info";
        core->dcpinfo.getKdmInfo();
    }
    int pos = file.lastIndexOf("/");
    QString now_title = file.mid(pos+1);
    tabwidget->setTabText(0,now_title);
    QString video_url = file + "/" + core->dcpinfo.video_url;
    QString audio_url = file + "/" + core->dcpinfo.audio_url;
    core->dcpinfo.video_url = video_url;
    core->dcpinfo.audio_url = audio_url;
    core->openDcp(video_url,audio_url,core->dcpinfo.key);
}

void DcpPlayer::createPanel()
{
    tabwidget = new QTabWidget(this);
    tabwidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    tabwidget->setFocusPolicy(Qt::StrongFocus);
    panel = new QWidget(this);
    panel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    panel->setMinimumSize(QSize(1,1));
    panel->setFocusPolicy(Qt::StrongFocus);
    panel->setObjectName("panel");
    panel->setStyleSheet("#panel{background-color:black}");
    tabwidget->addTab(panel,"player_tab");
    tabwidget->setStyleSheet("QTabWidget { border-top-color: rgba(255, 255, 255, 0); }");
    tabwidget->setDocumentMode(true);
}

void DcpPlayer::helpAbout(){
    About d(this);
    d.exec();
}

void DcpPlayer::createMplayerWindow()
{
    mplayerwindow = new MplayerWindow(panel);
    mplayerwindow->show();
    mplayerwindow->setObjectName("mplayerwindow");
    mplayerwindow->allowVideoMovement(false);
    mplayerwindow->delayLeftClick(false);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(mplayerwindow);
    panel->setLayout(layout);
    mplayerwindow->activateMouseDragTracking(true);

    connect(mplayerwindow,SIGNAL(doubleClicked()),
            this,SLOT(doubleClickFunction()));
    connect(mplayerwindow,SIGNAL(rightClicked()),
            this,SLOT(rightClickFunction()));
}

void DcpPlayer::disableActionsOnStop(){
    playOrPauseAct->setEnabled(true);
    stopAct->setEnabled(true);
    timeslider_action->setPos(0);
    time_label_action->setCurrentTime(0);
    time_label_action->setTotalTime(0);
    timeslider_action->setEnabled(false);
    volumeslider_action->setEnabled(false);
    fullScreenAct->setEnabled(false);
    forward1Act->setEnabled(false);
    forward2Act->setEnabled(false);
    forward3Act->setEnabled(false);
    rewind1Act->setEnabled(false);
    rewind2Act->setEnabled(false);
    rewind3Act->setEnabled(false);
    moveUpAct->setEnabled(false);
    moveDownAct->setEnabled(false);
    moveLeftAct->setEnabled(false);
    moveRightAct->setEnabled(false);
    incZoomAct->setEnabled(false);
    decZoomAct->setEnabled(false);
    resetZoomAct->setEnabled(false);
    autoZoomAct->setEnabled(false);
    autoZoom169Act->setEnabled(false);
    autoZoom235Act->setEnabled(false);
    decVolumeAct->setEnabled(false);
    incVolumeAct->setEnabled(false);
    incAudioDelayAct->setEnabled(false);
    decAudioDelayAct->setEnabled(false);
}

void DcpPlayer::gotCurrentTime(double sec){
    static int last_second = 0;
    if(floor(sec) == last_second)return;
    last_second = (int)floor(sec);

    QString time = Helper::formatTime((int)sec) + "/" +
                                      Helper::formatTime((int)core->dcpinfo.duration);
    emit timeChanged(sec);
    emit timeChanged(time);
}

void DcpPlayer::setMute(bool b){
    if(!b){
        muteAction->setIcon(Images::icon("volume"));
    }else{
        muteAction->setIcon(Images::icon("mute"));
    }
}

void DcpPlayer::exitFullscreen(){
    if(core->mset.fullscreen == true){
        toggleFullscreen(false);
    }
}

void DcpPlayer::toggleFullscreen(bool b){
    if(b == core->mset.fullscreen){
        qDebug("DcpPlayer::toggleFullscreen:returing");
        return;
    }
    core->mset.fullscreen = b;
    if(b){
        win_pos = pos();
        win_size = size();
        was_maximized = isMaximized();
        aboutToEnterFullscreen();
        showFullScreen();
    }else{
        showNormal();
        if(was_maximized)showMaximized();

        aboutToExitFullscreen();
        move(win_pos);
        resize(win_size);
    }
    setFocus();
}

void DcpPlayer::aboutToEnterFullscreen(){
    setStayOnTop(true);
    menuBar()->hide();
    tabwidget->tabBar()->hide();
    control_tools->hide();
    hide_widget->deactivate();
    QTimer::singleShot(100,hide_widget,SLOT(activate()));
    //panel->showFullScreen();
    //mplayerwindow->showFullScreen();
    //mplayerwindow->videoLayer()->showFullScreen();
    reconfigureFloatingControl();
    hide_widget->adjustSize();
//    hide_widget->adjustSize();
    //mplayerwindow->updateVideoWindow();
    //tabwidget->tabBarAutoHide();
}

void DcpPlayer::aboutToExitFullscreen(){
    menuBar()->show();
    tabwidget->tabBar()->show();
    hide_widget->deactivate();
    hide_widget->adjustSize();
    control_tools->show();
    //panel->showNormal();
    //mplayerwindow->showNormal();
    //mplayerwindow->videoLayer()->showNormal();
   // mplayerwindow->updateVideoWindow();
    setStayOnTop(false);
}

void DcpPlayer::reconfigureFloatingControl(){
    hide_widget->setMargin(0);
    hide_widget->setPercWidth(100);
}

void DcpPlayer::setStayOnTop(bool b){
    /*if((b && (windowFlags() & Qt::WindowStaysOnTopHint))||
            (!b && (!(windowFlags() & Qt::WindowStaysOnTopHint))))
    {
        qDebug("DcpPlayer::setstayontop:nothing to do");
        return;
    }*/
    bool visible = isVisible();
    QPoint old_pos = pos();

    if(b){
        setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);
        qDebug()<<"full";
    }
    else{
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        qDebug()<<"not full";
    }

    move(old_pos);

    if(visible){
        show();
    }
}

void DcpPlayer::resizeWindow(int w, int h){
    qDebug("DcpPlayer:resizewindow w=%d h=%d",w,h);
    if(core->mset.fullscreen)return;
    if(panel->isVisible()){
        return;
    }
    if(!panel->isVisible()){
        panel->show();
    }
    //resizeMainWindow(w,h);
    return;
}

void DcpPlayer::resizeMainWindow(int w, int h){
    if(core->mset.factor != 100){
        w = w * core->mset.factor / 100;
        h = h * core->mset.factor / 100;
    }
    QSize video_size(w,h);
    if(video_size == panel->size()){
        qDebug("DcpPlayer::resizeWindow:the panel size is already the size.Do nothing");
        return;
    }
    int diff_width = this->width() - panel->width();
    int diff_height = this->height() - panel->height();

    int new_width = w + diff_width;
    int new_height = h+ diff_height;

    qDebug("DcpPlayer::resizeWindow:new_width:%d new_height:%d",new_width,new_height);

    resize(new_width,new_height);
    QRect screen_rect = QApplication::desktop()->screenGeometry(this);
    QPoint right_bottom = QPoint(this->pos().x() + this->width(), this->pos().y() + this->height());
    qDebug("BaseGui::resizeWindow: right bottom point: %d, %d", right_bottom.x(), right_bottom.y());;
    if (!screen_rect.contains(right_bottom) || !screen_rect.contains(this->pos())) {
        qDebug("BaseGui::resizeWindow: the window is outside of the desktop, it will be moved");
        int x = screen_rect.x() + ((screen_rect.width() - width()) / 2);
        int y = screen_rect.y() + ((screen_rect.height() - height()) / 2);
        move(x, y);
    }
}

void DcpPlayer::centerWindow(){
    QRect r = QApplication::desktop()->screenGeometry(this);
    int x = r.x() + ((r.width() - width())/2);
    int y = r.y() + ((r.height() - height())/2);
    //move(x,y);
}

void DcpPlayer::displayGotoTime(int t){
    int jump_time = (int)core->dcpinfo.duration*t/1000;
    return;
}

void DcpPlayer::doubleClickFunction(){
    processFunction("fullscreen");
}

void DcpPlayer::rightClickFunction(){
    processFunction("rightmouse");
}

void DcpPlayer::processFunction(QString function){
    if(function == "fullscreen"){
        fullScreenAct->trigger();
    }
    else if(function == "rightmouse"){
        showContextMenuAct->trigger();
    }
}

void DcpPlayer::showExitCodeFromMplayer(int exit_code){
    qDebug("DcpPlayer::showExitCodeFromMplayer");
    qDebug("%d",exit_code);
}

void DcpPlayer::updateTabwidget(int index){
    qDebug("DcpPlayer::updateTabWidget: index=%d",index);
    if(index == 0){
        control_tools->show();
        tabwidget->setCurrentIndex(0);
    }
    else if(index == 1){
        control_tools->hide();
        tabwidget->setCurrentIndex(1);
    }
    if(core->dcpinfo.dcp_title != ""){
       tabwidget->setTabText(0,tr("正在播放---")+core->dcpinfo.dcp_title.toLatin1().data());
    }
}

void DcpPlayer::keyPressEvent(QKeyEvent *e){

    if(core->state() == Core::PLAYING){
        if(e->key() == Qt::Key_Right){
            qDebug()<<"left";
            forward1Act->trigger();
        }
        else if(e->key() == Qt::Key_Left){
            qDebug()<<"right";
            rewind1Act->trigger();
        }
    }
    e->accept();
}

void DcpPlayer::showPopupMenu(){
    showPopupMenu(QCursor::pos());
}

void DcpPlayer::showPopupMenu(QPoint p){
    popup->move(p);
    popup->show();
}

void DcpPlayer::closeWindow(){
    qDebug("DcpPlayer::closewindow");
    if(core->state() != Core::STOPPED){
        core->stop();
    }
    delete core;
    qDebug()<<wp->ischecked;
    if(wp->ischecked){
        delete wp;
    }
    qApp->quit();
}

void DcpPlayer::closeEvent(QCloseEvent *e){
    qDebug("DcpPlayer::closeevent");
    e->ignore();
    closeWindow();
}

void DcpPlayer::changeSizeFactor(int factor){
    //If fullscreen.don't resize!
    if(core->mset.fullscreen)return;

    resizeMainWindow(core->mset.win_width,core->mset.win_height);
}

void DcpPlayer::retranslateStrings(){
    //openMenu->setIcon(Images::icon("open"));
    setWindowIcon(Images::icon("logo",64));

    //Menus
    exitAct->change(Images::icon("close"),tr("关闭"));

    //Video Menu
    aspectMenu->menuAction()->setText(tr("屏幕比例"));
    aspectMenu->menuAction()->setIcon(Images::icon("aspect"));
    openDcpAct->setIcon(Images::icon("openfolder"));
    playOrPauseAct->setIcon(Images::icon("play"));
    fullScreenAct->setIcon(Images::icon("fullscreen"));
    aspectDetectAct->change(tr("&Auto"));
    aspect11Act->change("1&:1");
    aspect32Act->change("&3:2");
    aspect43Act->change("&4:3");
    aspect118Act->change("11:&8");
    aspect54Act->change("&5:4");
    aspect149Act->change("&14:9");
    aspect1410Act->change("1&4:10");
    aspect1610Act->change("1&6:10");
    aspect169Act->change("16:&9");
    aspect235Act->change("&2.35:1");
    aspectGroup->setActionsEnabled(false);

    zoom_menu->menuAction()->setText(tr("Zoo&m"));
    zoom_menu->menuAction()->setIcon(Images::icon("zoom"));
    resetZoomAct->change(tr("&Reset"));
    autoZoomAct->change(tr("&Auto zoom"));
    autoZoom169Act->change(tr("Zoom for &16:9"));
    autoZoom235Act->change(tr("Zoom for &2.35:1"));
    decZoomAct->change(tr("Zoom &-"));
    incZoomAct->change(tr("Zoom &+"));
    moveLeftAct->change(tr("Move &left"));
    moveRightAct->change(tr("Move &right"));
    moveUpAct->change(tr("Move &up"));
    moveDownAct->change(tr("Move &down"));

    //Audio Menu
    muteAction->setIcon(Images::icon("volume"));
    stopAct->setIcon(Images::icon("stop"));
    audiochannels_menu->menuAction()->setText("声道控制");
    audiochannels_menu->menuAction()->setIcon(Images::icon("audio_channels"));
    channelsStereoAct->change(tr("立体声"));
    channelsSurroundAct->change(tr("4.0声道"));
    channelsFull51Act->change(tr("5.1声道"));
    channelsFull61Act->change(tr("6.1声道"));
    channelsGroup->setActionsEnabled(false);
    stereomode_menu->menuAction()->setText("双声道选项");
    stereomode_menu->menuAction()->setIcon(Images::icon(("stereo_mode")));
    stereoAct->change(tr("立体声"));
    leftChannelAct->change(tr("左声道"));
    rightChannelAct->change(tr("右声道"));
    monoAct->change(tr("单声道"));
    reverseAct->change(tr("声道反转"));
    stereoGroup->setActionsEnabled(false);

    //Play Menu
    forward1Act->change(Images::icon("forward10s"),tr("快进10秒"));
    forward2Act->change(Images::icon("forward1m"),tr("快进1分钟"));
    forward3Act->change(Images::icon("forward10m"),tr("快进10分钟"));
    rewind1Act->change(Images::icon("rewind10s"),tr("快退10秒"));
    rewind2Act->change(Images::icon("rewind1m"),tr("快退1分钟"));
    rewind3Act->change(Images::icon("rewind10m"),tr("快退10分钟"));
    decVolumeAct->change(Images::icon("audio_down"),tr("降低音量"));
    incVolumeAct->change(Images::icon("audio_up"),tr("提升音量"));
    decAudioDelayAct->change(Images::icon("delay_down"),tr("延迟 -100毫秒"));
    incAudioDelayAct->change(Images::icon("delay_up"),tr("延迟 +100毫秒"));

    QPalette pe;
    pe.setColor(QPalette::Background,Qt::black);
    tabwidget->tabBar()->setPalette(pe);
    tabwidget->setTabText(0,tr("播放器"));
    fullScreenAct->setEnabled(false);
    muteAction->setEnabled(false);
    timeslider_action->setEnabled(false);
    volumeslider_action->setEnabled(false);
    forward1Act->setEnabled(false);
    forward2Act->setEnabled(false);
    forward3Act->setEnabled(false);
    rewind1Act->setEnabled(false);
    rewind2Act->setEnabled(false);
    rewind3Act->setEnabled(false);
    moveUpAct->setEnabled(false);
    moveDownAct->setEnabled(false);
    moveLeftAct->setEnabled(false);
    moveRightAct->setEnabled(false);
    incZoomAct->setEnabled(false);
    decZoomAct->setEnabled(false);
    resetZoomAct->setEnabled(false);
    autoZoomAct->setEnabled(false);
    autoZoom169Act->setEnabled(false);
    autoZoom235Act->setEnabled(false);
    decVolumeAct->setEnabled(false);
    incVolumeAct->setEnabled(false);
    incAudioDelayAct->setEnabled(false);
    decAudioDelayAct->setEnabled(false);
    //tabwidget stylesheet
    tabwidget->setStyleSheet("QTabBar::tab {background: qlineargradient(spread:pad, x1:1.5, y2:0, x2:1.5, y2:2, stop:0 rgba(73, 73, 74, 74), stop:1 rgba(40, 40, 40, 40));\
                             border: 1px solid rgb(190, 190, 190);\
                             max-height: 2.6em;\
                             min-width: 5.6em;\
                             padding: 8px;\
                             margin-left: -1px;\
                             margin-right: -1px;\
                            }"
                            "QTabBar::tab:selected, QTabBar::tab:hover {\
                             background: qlineargradient(spread:pad, x1:1.5, y1:2, x2:1.5, y2:0, stop:0 rgba(39, 117, 219, 255), stop:1 rgba(107, 171, 249, 255));\
                            }");
}

void DcpPlayer::playtest(){
    qDebug("DcpPlayer::playtest");
    core->playlist->clear();
    QString s = QFileDialog::getOpenFileName(this);
    if(s == "/"||s == "")
    {return;}
    qDebug()<<s;
    QFile nfsfile(s);
    if(!nfsfile.open(QIODevice::ReadOnly|QIODevice::Text)){
        return;
    }
    QTextStream in(&nfsfile);
    while(!in.atEnd()){
        QString dcpline = in.readLine();
        QString kdmline = in.readLine();
        dcpline.remove("[");
        dcpline.remove("]");
        kdmline.remove("[");
        kdmline.remove("]");
        core->testnum = core->testnum + 1;
        core->playlist->addFile(dcpline,kdmline);
    }
    core->nownum = 0;
    core->playTest(0);
}
