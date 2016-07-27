#include "global.h"
#include "preference.h"
#include "constants.h"
#include "paths.h"
#include <QApplication>
#include <QFile>
#include <QSettings>

QSettings * Global::settings = 0;
Preference * Global::pref = 0;

using namespace Global;


void Global::global_init(const QString &config_path){
    qDebug("global_init");

    if(!config_path.isEmpty()){
        Paths::setConfigPath(config_path);
    }

    if(Paths::iniPath().isEmpty()){
        settings = new QSettings(QSettings::IniFormat,QSettings::UserScope,
                                 QString(COMPANY),QString(PROGRAM));
    }else{
        QString filename = Paths::iniPath() + "/BestMoviePlayer.ini";
        settings = new QSettings(filename,QSettings::IniFormat);
        qDebug("global_init::config file:'%s",filename.toUtf8().data());
    }

    pref = new Preference();
}

void Global::global_end(){
    qDebug("global_end");

    delete pref;
    pref = 0;

    delete settings;
}
