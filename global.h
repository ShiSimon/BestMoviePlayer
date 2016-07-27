#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

class QSettings;
class Preference;

namespace Global {
    extern QSettings *settings;

    extern Preference * pref;

    void global_init(const QString & config_path);
    void global_end();

}

#endif // GLOBAL_H
