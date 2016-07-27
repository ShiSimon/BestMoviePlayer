#ifndef PATHS_H
#define PATHS_H

#include <QString>

class Paths
{

public:

    static void setAppPath(QString path);
    static QString appPath();

    static QString dataPath();
    static QString translationPath();
    static QString docPath();
    static QString themesPath();
    static QString doc(QString file,QString locale = QString::null,bool english_fallback = true);

    //!Forces to use a different path for the config file
    static void setConfigPath(QString path);

    //Return the Path where should save its config files
    static QString configPath();

    //just return configPath
    static QString iniPath();

private:
    static QString app_path;
    static QString config_path;

};

#endif // PATHS_H
