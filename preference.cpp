#include "preference.h"

#include <QSettings>
#include <QFileInfo>
#include <QRegExp>
#include <QDir>
#include <QLocale>
#include <QDesktopServices>
#include <QStandardPaths>

#include "global.h"

using namespace Global;

Preference::Preference()
{
    reset();

    load();
}

Preference::~Preference(){
    save();

}

void Preference::reset(){
    //General
    mplayer_bin = "mplayer/bin/mplayer.exe";
    mainpage = "http://10.1.0.102:8081/bestv-cinema-index/movie_param";
    ip_domain = "";

    //LOG
#ifdef LOG_BESTMOVIEPLAYER
    log_bestmovieplayer = true;
    log_filter = ".*";
    save_bmplayer_log = true;

#endif
}

void Preference::save(){
    qDebug("Preference::save");

    QSettings *set = settings;

    //General

    set->beginGroup("General");

    set->setValue("mplayer_bin",mplayer_bin);
    set->setValue("MainPage",mainpage);
    set->setValue("ip_domain",ip_domain);
    set->endGroup();

    //LOG
    set->beginGroup("Log");
#ifdef LOG_BESTMOVIEPLAYER
    set->setValue("log_bmplayer",log_bestmovieplayer);
    set->setValue("log_filter",log_filter);
    set->setValue("save_bmplayer_log",save_bmplayer_log);
#endif
    set->endGroup();


    set->sync();
}

void Preference::load(){
    qDebug("Preference::load");

    QSettings * set = settings;


    //General

    set->beginGroup("General");

    mplayer_bin = set->value("mplayer_bin",mplayer_bin).toString();
    mainpage = set->value("MainPage",mainpage).toString();
    ip_domain = set->value("ip_domain",ip_domain).toString();
    //LOG
    set->beginGroup("Log");
#ifdef LOG_BESTMOVIEPLAYER
    log_bestmovieplayer = set->value("log_bmplayer",log_bestmovieplayer).toBool();
    log_filter = set->value("log_filter",log_filter).toString();
    save_bmplayer_log = set->value("save_bmplayer_log",save_bmplayer_log).toBool();
#endif
    set->endGroup();

    set->endGroup();
}
