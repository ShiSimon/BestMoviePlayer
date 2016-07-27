#ifndef BESTMOVIEPLAYER_H
#define BESTMOVIEPLAYER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFile>
#include "dcpplayer.h"

class BestMoviePlayer : public QObject
{
    Q_OBJECT

public:
    enum ExitCode{ErrorArgument = -3,NoAction = -2,NoRunningInstance = -1,NoError = 0,NoExit = 1};

    BestMoviePlayer(const QString & config_path = QString::null,QObject *parent = 0);
    ~BestMoviePlayer();

    DcpPlayer *main_window;

    //!Process arguments.If ExitCode != NoExit the application must be exited.
    ExitCode processArgs(QStringList args);

    void start();

private slots:

private:

    void createConfigDirectory();

    QString actions_list;
    QString gui_to_use;
    QString media_title;

    //Change position and size
    bool move_gui;
    QPoint gui_position;

    bool resize_gui;
    QSize gui_size;

    //Options to pass to gui
    int close_at_end;//-1 = no set,1 = true,0 false
    int start_in_fullscreen;//-1 = no set,1 = true,0 false

#ifdef LOG_BESTMOVIEPLAYER
    static QFile output_log;
    #if QT_VERSION >= 0x050000
    static void myMessageOutput(QtMsgType type,const QMessageLogContext &,const QString & msg);
    #else
    static void myMessageOutput(QtMsgType type,const char *msg);
    #endif
    static bool allow_to_send_log_to_gui;
#endif
};

#endif // BESTMOVIEPLAYER_H
