#ifndef DCPPLAYER_H
#define DCPPLAYER_H

#include <QMainWindow>
#include <QProcess>
#include "dcpdata.h"
#include "core.h"

class QWidget;
class QMenu;
class QLabel;
class MplayerWindow;
class AutoHideWidget;
class MyAction;
class TimeSliderAction;
class TimeLabelAction;
class VolumeSliderAction;
class QToolBar;
class WebPage;
class QKeyEvent;
class MyActionGroup;

class DcpPlayer : public QMainWindow
{
    Q_OBJECT

public:
    DcpPlayer(QWidget *parent = 0,Qt::WindowFlags flags = 0);
    ~DcpPlayer();

public slots:
    virtual void openDcp();
    virtual void openDcp(QString file);
    virtual void playtest();
    //virtual void exitFullscreen();
    //virtual void toggleFullscreen();
    virtual void toggleFullscreen(bool);
    void exitFullscreen();
    void setStayOnTop(bool b);
    virtual void gotCurrentTime(double);
    void updateWidget(Core::State states);
    void setMute(bool b);
    virtual void helpAbout();

signals:
    void timeChanged(double current_time);
    void timeChanged(QString time_ready_to_print);

private slots:

protected slots:
    void changeSizeFactor(int factor);
    virtual void closeWindow();
    virtual void disableActionsOnStop();
    void resizeWindow(int w,int h);
    void resizeMainWindow(int w,int h);
    void centerWindow();
    void displayGotoTime(int t);

    void doubleClickFunction();
    virtual void rightClickFunction();
    void processFunction(QString function);
    void showExitCodeFromMplayer(int exit_code);
    void updateTabwidget(int index);
    virtual void showPopupMenu();
    virtual void showPopupMenu(QPoint p);

protected:
    void createCore();
    void createPanel();
    void createMplayer();
    void createActions();
    void createMenus();
    void createMplayerWindow();
    void createToolBar();
    void createHideBar();
    void aboutToEnterFullscreen();
    void aboutToExitFullscreen();
    void reconfigureFloatingControl();

    virtual void keyPressEvent(QKeyEvent *e);
    virtual void closeEvent(QCloseEvent *e);
    void retranslateStrings();

protected:
    QWidget *panel;
    QToolBar *control_tools;
    QTabWidget *tabwidget;
    MplayerWindow *mplayerwindow;
    QProcess *mplayer;
    QToolBar *control_widget;
    MyAction *playOrPauseAct;
    MyAction *stopAct;
    MyAction *fullScreenAct;
    MyAction *rewind1Act;
    MyAction *rewind2Act;
    MyAction *rewind3Act;
    MyAction *forward1Act;
    MyAction *forward2Act;
    MyAction *forward3Act;
    MyAction *muteAction;
    MyAction *exitFullscreenAct;
    MyAction *aboutThisAct;

    MyAction *showContextMenuAct;

    //Menu File
    MyAction *openDcpAct;
    MyAction *testDcp;
    MyAction *exitAct;

    //Aspect Action Group
    MyActionGroup *aspectGroup;
    MyAction *aspectDetectAct;
    MyAction *aspect11Act;
    MyAction *aspect32Act;
    MyAction *aspect43Act;
    MyAction *aspect118Act;
    MyAction *aspect54Act;
    MyAction *aspect149Act;
    MyAction *aspect1410Act;
    MyAction *aspect169Act;
    MyAction *aspect1610Act;
    MyAction *aspect235Act;

    //Videosize Group
    MyActionGroup *sizeGroup;
    MyAction *size50;
    MyAction *size75;
    MyAction *size100;
    MyAction *size125;
    MyAction *size150;
    MyAction *size175;
    MyAction *size200;
    MyAction *size300;
    MyAction *size400;

    //Video zoom
    MyAction *moveUpAct;
    MyAction *moveDownAct;
    MyAction *moveLeftAct;
    MyAction *moveRightAct;
    MyAction *incZoomAct;
    MyAction *decZoomAct;
    MyAction *resetZoomAct;
    MyAction *autoZoomAct;
    MyAction *autoZoom169Act;
    MyAction *autoZoom235Act;

    //Audio Channels Action Group
    MyActionGroup * channelsGroup;

    MyAction *channelsStereoAct;
    MyAction *channelsSurroundAct;
    MyAction *channelsFull51Act;
    MyAction *channelsFull61Act;

    MyActionGroup *stereoGroup;
    MyAction *stereoAct;
    MyAction *leftChannelAct;
    MyAction *rightChannelAct;
    MyAction *monoAct;
    MyAction *reverseAct;

    MyAction *decVolumeAct;
    MyAction *incVolumeAct;
    MyAction *decAudioDelayAct;
    MyAction *incAudioDelayAct;


    TimeLabelAction *time_label_action;
    TimeSliderAction *timeslider_action;
    VolumeSliderAction *volumeslider_action;
    AutoHideWidget *hide_widget;

    //MENUS
    QMenu *openMenu;
    QMenu *playMenu;
    QMenu *videoMenu;
    QMenu *audioMenu;
    QMenu *helpMenu;
    QMenu *popup;

    //other Menus
    QMenu *aspectMenu;
    QMenu *videosize_menu;
    QMenu *zoom_menu;
    QMenu *audiochannels_menu;
    QMenu *stereomode_menu;

    Core *core;
    WebPage *wp;

private:
    QPoint win_pos;
    QSize win_size;
    bool was_maximized;
};

#endif // DCPPLAYER_H
