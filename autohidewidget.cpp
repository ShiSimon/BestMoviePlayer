#include "autohidewidget.h"
#include <QTimer>
#include <QEvent>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QAction>
#include <QDebug>
#include <QMenu>


AutoHideWidget::AutoHideWidget(QWidget *parent)
    : QWidget(parent)
    ,turned_on(false)
    ,auto_hide(false)
    ,use_animation(false)
    ,spacing(0)
    ,perc_width(100)
    ,activation_area(Bottom)
    ,internal_widget(0)
    ,timer(0)
{
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);
    setLayoutDirection(Qt::LeftToRight);

    QWidget *widget_to_watch = parent;
    widget_to_watch->installEventFilter(this);
    installFiliter(widget_to_watch);

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(checkUnderMouse()));
    timer->setInterval(3000);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
}

AutoHideWidget::~AutoHideWidget(){}

void AutoHideWidget::setInternalWidget(QWidget *w){
    layout()->addWidget(w);
    internal_widget = w;
}

void AutoHideWidget::setHideDelay(int ms){
    timer->setInterval(ms);
}

int AutoHideWidget::hideDelay(){
    return timer->interval();
}

void AutoHideWidget::installFiliter(QObject *o){
    QObjectList children = o->children();
    for(int n=0;n < children.count();n++){
        if(children[n]->isWidgetType()){
            qDebug()<<"AutoHideWidget::installFilter:child name:"<<children[n]->objectName();
            QWidget *w = static_cast<QWidget*>(children[n]);
            w->setMouseTracking(true);
            w->installEventFilter(this);
            installFiliter(children[n]);
        }
    }
}

bool AutoHideWidget::visiblePopups() {
    //qDebug() << "AutohideWidget::visiblePopups: internal_widget:" << internal_widget;
    if (!internal_widget) return false;

    // Check if any of the menus in the internal widget is visible
    QObjectList children = internal_widget->children();
    foreach(QObject * child, children) {
        if (child->isWidgetType()) {
            //qDebug() << "AutohideWidget::visiblePopups:" << child << "child name:" << child->objectName();
            QWidget *w = static_cast<QWidget *>(child);

            QList<QAction *> actions = w->actions();
            foreach(QAction * action, actions) {
                //qDebug() << "AutohideWidget::visiblePopups: action:" << action;

                QList<QWidget *> aw = action->associatedWidgets();
                //qDebug() << "AutohideWidget::visiblePopups: aw:" << aw;

                QMenu * menu = 0;
                foreach(QWidget * widget, aw) {
                    //qDebug() << "AutohideWidget::visiblePopups: widget:" << widget;
                    if ((menu = qobject_cast<QMenu *>(widget))) {
                        //qDebug() << "AutohideWidget::visiblePopups: menu:" << menu << "visible:" << menu->isVisible();
                        if (menu->isVisible()) return true;
                    }
                }

                menu = action->menu();
                if (menu) {
                    //qDebug() << "AutohideWidget::visiblePopups: menu:" << menu << "visible:" << menu->isVisible();
                    if (menu->isVisible()) return true;
                }
            }
        }
    }
    return false;
}

void AutoHideWidget::activate() {
    turned_on = true;
    timer->start();
}

void AutoHideWidget::deactivate() {
    turned_on = false;
    timer->stop();
    hide();
}

void AutoHideWidget::show() {
    qDebug() << "AutohideWidget::show";
    resizeAndMove();

    if (use_animation) {
        showAnimated();
    } else {
        QWidget::show();
    }

    // Restart timer
    if (timer->isActive()) timer->start();
}

void AutoHideWidget::setAutoHide(bool b) {
    auto_hide = b;
}

void AutoHideWidget::checkUnderMouse() {
    if (auto_hide) {
        if (isVisible() && !underMouse() && !visiblePopups()) {
            hide();
        }
    }
}

void AutoHideWidget::resizeAndMove() {
    QWidget * widget = parentWidget();
    int w = widget->width() * perc_width / 100;
    int h = height();
    resize(w, h);

    int x = (widget->width() - width() ) / 2;
    int y = widget->height() - height() - spacing;
    move(x, y);
}

bool AutoHideWidget::eventFilter(QObject * obj, QEvent * event) {
    if (turned_on) {
        if (event->type() == QEvent::MouseMove) {
            //qDebug() << "AutohideWidget::eventFilter: mouse move" << obj;
            if (!isVisible()) {
                if (activation_area == Anywhere) {
                    show();
                } else {
                    QMouseEvent * mouse_event = dynamic_cast<QMouseEvent*>(event);
                    QWidget * parent = parentWidget();
                    QPoint p = parent->mapFromGlobal(mouse_event->globalPos());
                    //qDebug() << "AutohideWidget::eventFilter: y:" << p.y();
                    if (p.y() > (parent->height() - height() - spacing)) {
                        show();
                    }
                }
            }
        }
        else
        if (event->type() == QEvent::MouseButtonRelease && obj == this) {
            event->setAccepted(true);
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void AutoHideWidget::showAnimated() {
    QPoint initial_position = QPoint(pos().x(), parentWidget()->size().height());
    QPoint final_position = pos();
    move(initial_position);

    QWidget::show();
}

