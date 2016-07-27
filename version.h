#ifndef VERSION_H
#define VERSION_H

#include <QString>

class Version
{
public:
    static QString printable();
    static QString stable();
    static QString revision();
    static QString with_revision(){return stable() + "." + revision();}
};

#endif // VERSION_H
