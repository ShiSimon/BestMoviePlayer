#include "bestmovieplayer.h"
#include "global.h"
#include "preference.h"
#include "paths.h"
#include "config.h"
#include "myapplication.h"
//#include "clhelp.h"
#include "version.h"

#include <QDir>
#include <QUrl>
#include <QTime>
#include <stdio.h>
#include <QDebug>

using namespace Global;

BestMoviePlayer::BestMoviePlayer(const QString & config_path,QObject * parent)
    : QObject(parent)
{
#ifdef LOG_BESTMOVIEPLAYER
    #if QT_VERSION >= 0x050000
    qInstallMessageHandler(BestMoviePlayer::myMessageOutput);
    #else
    qInstallMessageHandler(BestMoviePlayer::myMessageOutput);
    #endif
    allow_to_send_log_to_gui = true;
#endif

    gui_to_use = "DefaultGUI";

    close_at_end = -1; //Not set
    start_in_fullscreen = -1; //Not Set

    move_gui = false;
    resize_gui = false;

    qDebug()<<"config_path = "<<config_path;
    Paths::setAppPath(qApp->applicationDirPath());

    global_init(config_path);

//    showInfo();
}

BestMoviePlayer::~BestMoviePlayer()
{

    global_end();

    #ifdef LOG_BESTMOVIEPLAYER
        if(output_log.isOpen()) output_log.close();
    #endif
}

void BestMoviePlayer::start(){
    main_window = new DcpPlayer();
    main_window->show();
}

BestMoviePlayer::ExitCode BestMoviePlayer::processArgs(QStringList args){
    qDebug("BestMoviePlayer::processArgs");
    MyApplication *a = MyApplication::instance();
    if(a->isRunning()){
        a->sendMessage("Hello");
        return NoError;
    }
    return NoExit;
}


#ifdef LOG_BESTMOVIEPLAYER
QFile BestMoviePlayer::output_log;
bool BestMoviePlayer::allow_to_send_log_to_gui = false;

#if QT_VERSION >= 0x050000
void BestMoviePlayer::myMessageOutput( QtMsgType type, const QMessageLogContext &, const QString & msg ) {
#else
void BestMoviePlayer::myMessageOutput( QtMsgType type, const char *msg ) {
#endif
    static QStringList saved_lines;
    static QString orig_line;
    static QString line2;
    static QRegExp rx_log;

    rx_log.setPattern(".*");

    line2.clear();

#if QT_VERSION >= 0x050000
    orig_line = msg;
#else
#ifdef Q_OS_WIN
    orig_line = QString::fromLocal8Bit(msg);
#else
    orig_line = QString::fromUtf8(msg);
#endif
#endif

    switch ( type ) {
    case QtDebugMsg:
        if (rx_log.indexIn(orig_line) > -1) {
            line2 = orig_line;
        }
        break;
    case QtWarningMsg:
        line2 = "WARNING: " + orig_line;
        break;
    case QtFatalMsg:
        line2 = "FATAL: " + orig_line;
        abort();                    // deliberately core dump
    case QtCriticalMsg:
        line2 = "CRITICAL: " + orig_line;
        break;
    }

    if (line2.isEmpty()) return;

    line2 = "["+ QTime::currentTime().toString("hh:mm:ss:zzz") +"] "+ line2;

    if (pref) {
        // Save log to file
        if (!output_log.isOpen()) {
            // FIXME: the config path may not be initialized if USE_LOCKS is not defined
            output_log.setFileName( Paths::appPath() + "/log/bmplayer_log.txt" );
            output_log.open(QIODevice::WriteOnly);
        }
        if (output_log.isOpen()) {
            QString l = line2 + "\r\n";
            output_log.write(l.toUtf8().constData());
            output_log.flush();
        }
    }
}
#endif
