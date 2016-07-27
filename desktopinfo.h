#ifndef DESKTOPINFO_H
#define DESKTOPINFO_H
#include <QSize>

class QWidget;

class DesktopInfo
{
public:
    static QSize desktop_size(QWidget *w);
    static double desktop_aspectRatio(QWidget *w);

    //! Returns true if the widget is inside the current screen
    static bool isInsideScreen(QWidget *w);
};

#endif // DESKTOPINFO_H
