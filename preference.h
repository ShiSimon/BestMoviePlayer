#ifndef PREFERENCE_H
#define PREFERENCE_H

#include <QString>
#include <QStringList>
#include <QSize>
#include "config.h"

class Preference
{
public:
    Preference();
    virtual ~Preference();

    virtual void reset();

    void save();
    void load();

    //General
    QString mplayer_bin;
    QString mainpage;
    QString ip_domain;

    //Logs
#ifdef LOG_BESTMOVIEPLAYER
    bool log_bestmovieplayer;
    QString log_filter;
    bool save_bmplayer_log;
#endif
};

#endif // PREFERENCE_H
