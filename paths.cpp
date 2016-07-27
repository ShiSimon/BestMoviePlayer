#include "paths.h"
#include <QLibraryInfo>
#include <QLocale>
#include <QFile>
#include <QRegExp>
#include <QDir>
#include <stdlib.h>

QString Paths::app_path;
QString Paths::config_path;


void Paths::setAppPath(QString path){
    app_path = path;
}

QString Paths::appPath(){
    return app_path;
}

QString Paths::dataPath(){
    return appPath();
}

QString Paths::docPath(){
    return appPath()+ "/doc";
}

QString Paths::themesPath(){
    return appPath() + "/themes";
}

void Paths::setConfigPath(QString path){
    config_path = path;
}

QString Paths::configPath(){
    if(!config_path.isEmpty()){
        return config_path;
    }

    return QDir::homePath() + "./BestMoviePlayer";
}

QString Paths::iniPath(){
    return configPath();
}
