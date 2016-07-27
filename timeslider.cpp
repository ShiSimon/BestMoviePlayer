#include "timeslider.h"

#include <QWheelEvent>
#include <QTimer>
#include <QToolTip>
#include <QDebug>

#include "helper.h"

#define DEBUG 0

TimeSlider::TimeSlider(QWidget *parent):MySlider(parent)
  ,dont_update(false)
  ,position(0)
  ,total_time(0)
{
    setMaximum(1000);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    connect(this,SIGNAL(sliderPressed()),this,SLOT(stopUpdate()));
    connect(this,SIGNAL(sliderReleased()),this,SLOT(resumeUpdate()));
    connect(this,SIGNAL(sliderReleased()),this,SLOT(mouseReleased()));
    connect(this,SIGNAL(valueChanged(int)),this,SLOT(valueChanged_slot(int)));
    connect(this,SIGNAL(draggingPos(int)),this,SLOT(checkDragging(int)));

    setStyleSheet("QSlider::groove:horizontal {border: 1px solid #bbb;background: white;height: 10px;\
                  border-radius: 4px;}""QSlider::handle:horizontal {background: qlineargradient(x1:0, y1:0, x2:1, y2:1,\
    stop:0 #eee, stop:1 #ccc);border: 1px solid #777;width: 13px;margin-top: -2px;margin-bottom: -2px;\
    border-radius: 4px;}""QSlider::handle:horizontal:hover {\
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,stop:0 #fff, stop:1 #ddd);\
    border: 1px solid #444;border-radius: 4px;}""QSlider::add-page:horizontal {\
    background: #fff;border: 1px solid #777;height: 10px;border-radius: 4px;}"
    "QSlider::sub-page:horizontal {background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1,\
    stop: 0 #66e, stop: 1 #bbf);background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1,stop: 0 #bbf, stop: 1 #55f);\
    border: 1px solid #777;height: 10px;border-radius: 4px;}"
    "QSlider::add-page:horizontal {background: #fff;border: 1px solid #777;height: 10px;border-radius: 4px;}"
    "QSlider::sub-page:horizontal:disabled {background: #bbb;border-color: #999;}"
    "QSlider::add-page:horizontal:disabled {background: #eee;border-color: #999;}");
    last_pos_to_send = -1;
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(sendDelayedPos()));
    timer->start(200);
}

TimeSlider::~TimeSlider(){
}

void TimeSlider::stopUpdate(){
    #if DEBUG
    QDebug("TimeSlider::stopUpdate");
    #endif
    dont_update = true;
}

void TimeSlider::resumeUpdate(){
    #if DEBUG
    QDebug("TimeSlider::resumeUpdate");
    #endif
    dont_update = false;
}

void TimeSlider::mouseReleased(){
#if DEBUG
    QDebug("TimeSlider::resumeUpdate");
#endif
    emit posChanged(value());
}

void TimeSlider::valueChanged_slot(int v) {
    #if DEBUG
    qDebug("TimeSlider::changedValue_slot: %d", v);
    #endif

    // Only to make things clear:
    bool dragging = dont_update;
    if (!dragging) {
        if (v!=position) {
            #if DEBUG
            qDebug(" emitting posChanged");
            #endif
            emit posChanged(v);
        }
    } else {
        #if DEBUG
        qDebug(" emitting draggingPos");
        #endif
        emit draggingPos(v);
    }
}

void TimeSlider::setDragDelay(int d) {
    qDebug("TimeSlider::setDragDelay: %d", d);
    timer->setInterval(d);
}

int TimeSlider::dragDelay() {
    return timer->interval();
}

void TimeSlider::checkDragging(int v) {
    qDebug("TimeSlider::checkDragging: %d", v);
    last_pos_to_send = v;
}

void TimeSlider::sendDelayedPos() {
    if (last_pos_to_send != -1) {
        qDebug("TimeSlider::sendDelayedPos: %d", last_pos_to_send);
        emit delayedDraggingPos(last_pos_to_send);
        last_pos_to_send = -1;
    }
}


void TimeSlider::setPos(int v) {
    #if DEBUG
    qDebug("TimeSlider::setPos: %d", v);
    qDebug(" dont_update: %d", dont_update);
    #endif

    if (v!=pos()) {
        if (!dont_update) {
            position = v;
            setValue(v);
        }
    }
}

int TimeSlider::pos() {
    return position;
}

void TimeSlider::wheelEvent(QWheelEvent * e) {
    //e->ignore();

    qDebug("TimeSlider::wheelEvent: delta: %d", e->delta());
    e->accept();

    if (e->orientation() == Qt::Vertical) {
        if (e->delta() >= 0)
            emit wheelUp();
        else
            emit wheelDown();
    } else {
        qDebug("Timeslider::wheelEvent: horizontal event received, doing nothing");
    }
}

bool TimeSlider::event(QEvent *event) {
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent * help_event = static_cast<QHelpEvent *>(event);
        qDebug() << "TimeSlider::event: total_time:" << total_time << "x:" << help_event->x();
        int pos_in_slider = help_event->x() * maximum() / width();
        int time = pos_in_slider * total_time / maximum();
        //qDebug() << "TimeSlider::event: time:" << time;
        if (time >= 0 && time <= total_time) {
            QToolTip::showText(help_event->globalPos(), Helper::formatTime(time), this);
        } else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
    return QWidget::event(event);
}
