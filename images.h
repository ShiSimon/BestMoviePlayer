#ifndef IMAGES_H
#define IMAGES_H

#include <QString>
#include <QPixmap>
#include <QIcon>

class Images
{
public:
    static void setThemesPath(const QString & folder);

    static QPixmap icon(QString name,int size=-1);

    static QString file(const QString & name);
    static bool has_rcc;

private:
    static QPixmap resize(QPixmap *p,int size = 20);

    static QString current_theme;
    static QString themes_path;
    static QString resourceFilename();
};

#endif // IMAGES_H
