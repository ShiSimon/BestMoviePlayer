#include "version.h"

#define VERSION "1.0.0"

#ifdef Q_OS_WIN
#if defined(_WIN64)
#define BMPWIN_ARCH "(64-bit)"
#elif defined(_WIN32) && !defined(WIN64)
#define BMPWIN_ARCH "(32-bit)"
#endif
#endif

QString Version::printable()
{

    return QString(QString(VERSION)) + " " + QString(BMPWIN_ARCH);
}

QString Version::stable(){
    return QString(VERSION);
}

QString Version::revision(){
    return QString(VERSION);
}
