#include "desktopinfo.h"
#include <QApplication>
#include <QDesktopWidget>

QSize DesktopInfo::desktop_size(QWidget *w) {
    QDesktopWidget * dw = QApplication::desktop();
    qDebug("DesktopInfo::desktop_size: primary screen: %d", dw->primaryScreen());

    QSize s = dw->screen( dw->primaryScreen() )->size();

    qDebug("DesktopInfo::desktop_size: size of primary screen: %d x %d", s.width(), s.height() );
    //return dw->screen( dw->primaryScreen() )->size();

    QRect r = dw->screenGeometry(w);
    qDebug("DesktopInfo::desktop_size: size of screen: %d x %d", r.width(), r.height() );

    return QSize(r.width(), r.height() );
}

double DesktopInfo::desktop_aspectRatio(QWidget *w) {
    QSize s = DesktopInfo::desktop_size(w);
    return  (double) s.width() / s.height() ;
}

bool DesktopInfo::isInsideScreen(QWidget *w) {
    QDesktopWidget * dw = QApplication::desktop();
    QRect r = dw->screenGeometry(w);

    qDebug("DesktopInfo::isInsideScreen: geometry of screen: x:%d y:%d w:%d h:%d", r.x(), r.y(), r.width(), r.height() );
    return r.contains(w->pos());
}
