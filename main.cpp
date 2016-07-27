#include "bestmovieplayer.h"
#include "myapplication.h"

#include <QDir>
#include <QApplication>

int main(int argc, char ** argv)
{
    MyApplication a("BestMoviePlayer",argc, argv);

    a.setQuitOnLastWindowClosed(false);

    //a.addLibraryPath("plugins");

//    a.addLibraryPath("libs");

#ifdef Q_OS_WIN
    //Change the working directory to the application path
    QDir::setCurrent(a.applicationDirPath());
#endif

    //Enable icons in menus
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus,false);

    //Sets the config_path
    QString config_path;

    config_path = a.applicationDirPath() + "/config";

    //If a BestMoviePlayer.ini exits in the app path,will use the path
    if(QFile::exists(a.applicationDirPath() + "/BestMoviePlayer.ini")){
        config_path = a.applicationDirPath();
        qDebug("main::using existing %s",QString(config_path + "/BestMoviePlayer.ini").toUtf8().data());
    }

    QStringList args = a.arguments();


    BestMoviePlayer *bestmovieplayer = new BestMoviePlayer(config_path);

    BestMoviePlayer::ExitCode c = bestmovieplayer->processArgs(args);
    if(c != BestMoviePlayer::NoExit){
        return c;
    }

    bestmovieplayer->start();

    int r = a.exec();

    delete bestmovieplayer;

    return r;
}
