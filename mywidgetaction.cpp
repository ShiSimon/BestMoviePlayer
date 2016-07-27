#include "mywidgetaction.h"
#include <QDebug>
#include <QLabel>
#include "helper.h"

MyWidgetAction::MyWidgetAction(QWidget * parent)
    :QWidgetAction(parent)
{
    custom_style = 0;
    custom_stylesheet = "";
}

MyWidgetAction::~MyWidgetAction() {
}

void MyWidgetAction::enable() {
    propagate_enabled(true);
}

void MyWidgetAction::disable() {
    propagate_enabled(false);
}

void MyWidgetAction::propagate_enabled(bool b) {
    QList<QWidget *> l = createdWidgets();
    for (int n=0; n < l.count(); n++) {
        l[n]->setEnabled(b);
    }
    setEnabled(b);
}


TimeSliderAction::TimeSliderAction( QWidget * parent )
    : MyWidgetAction(parent)
{
    drag_delay = 200;
}

TimeSliderAction::~TimeSliderAction() {
}

void TimeSliderAction::setPos(int v) {
    QList<QWidget *> l = createdWidgets();
    for (int n=0; n < l.count(); n++) {
        TimeSlider *s = (TimeSlider*) l[n];
        bool was_blocked= s->blockSignals(true);
        s->setPos(v);
        s->blockSignals(was_blocked);
    }
}

int TimeSliderAction::pos() {
    QList<QWidget *> l = createdWidgets();
    if (l.count() >= 1) {
        TimeSlider *s = (TimeSlider*) l[0];
        return s->pos();
    } else {
        return -1;
    }
}

void TimeSliderAction::setDuration(double t) {
    qDebug() << "TimeSliderAction::setDuration:" << t;
    total_time = t;
    QList<QWidget *> l = createdWidgets();
    for (int n=0; n < l.count(); n++) {
        TimeSlider *s = (TimeSlider*) l[n];
        s->setDuration(t);
    }
}

void TimeSliderAction::setDragDelay(int d) {
    drag_delay = d;

    QList<QWidget *> l = createdWidgets();
    for (int n=0; n < l.count(); n++) {
        TimeSlider *s = (TimeSlider*) l[n];
        s->setDragDelay(drag_delay);
    }
}

int TimeSliderAction::dragDelay() {
    return drag_delay;
}

QWidget * TimeSliderAction::createWidget ( QWidget * parent ) {
    TimeSlider *t = new TimeSlider(parent);
    t->setEnabled( isEnabled() );

    if (custom_style) t->setStyle(custom_style);
    if (!custom_stylesheet.isEmpty()) t->setStyleSheet(custom_stylesheet);

    connect( t,    SIGNAL(posChanged(int)),
             this, SIGNAL(posChanged(int)) );
    connect( t,    SIGNAL(draggingPos(int)),
             this, SIGNAL(draggingPos(int)) );
    t->setDragDelay(drag_delay);

    connect( t,    SIGNAL(delayedDraggingPos(int)),
             this, SIGNAL(delayedDraggingPos(int)) );

    connect(t, SIGNAL(wheelUp()), this, SIGNAL(wheelUp()));
    connect(t, SIGNAL(wheelDown()), this, SIGNAL(wheelDown()));

    return t;
}

VolumeSliderAction::VolumeSliderAction( QWidget * parent )
    : MyWidgetAction(parent)
{
    tick_position = QSlider::TicksBelow;
}

VolumeSliderAction::~VolumeSliderAction() {
}

void VolumeSliderAction::setValue(int v) {
    QList<QWidget *> l = createdWidgets();
    for (int n=0; n < l.count(); n++) {
        MySlider *s = (MySlider*) l[n];
        bool was_blocked = s->blockSignals(true);
        s->setValue(v);
        s->blockSignals(was_blocked);
    }
}

int VolumeSliderAction::value() {
    QList<QWidget *> l = createdWidgets();
    if (l.count() >= 1) {
        MySlider *s = (MySlider*) l[0];
        return s->value();
    } else {
        return -1;
    }
}

void VolumeSliderAction::setTickPosition(QSlider::TickPosition position) {
    // For new widgets
    tick_position = position;

    // Propagate changes to all existing widgets
    QList<QWidget *> l = createdWidgets();
    for (int n=0; n < l.count(); n++) {
        MySlider *s = (MySlider*) l[n];
        s->setTickPosition(tick_position);
    }
}

QWidget * VolumeSliderAction::createWidget ( QWidget * parent ) {
    MySlider *t = new MySlider(parent);

    if (custom_style) t->setStyle(custom_style);
    if (!custom_stylesheet.isEmpty()) t->setStyleSheet(custom_stylesheet);
    if (fixed_size.isValid()) t->setFixedSize(fixed_size);

    t->setMinimum(0);
    t->setMaximum(100);
    t->setValue(80);
    t->setOrientation( Qt::Horizontal );
    t->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    t->setFocusPolicy( Qt::NoFocus );
    t->setTickPosition( tick_position );
    t->setTickInterval( 10 );
    t->setSingleStep( 1 );
    t->setPageStep( 10 );
    t->setToolTip( tr("Volume") );
    t->setEnabled( isEnabled() );
    t->setAttribute(Qt::WA_NoMousePropagation);

    connect( t,    SIGNAL(valueChanged(int)),
             this, SIGNAL(valueChanged(int)) );
    return t;
}

TimeLabelAction::TimeLabelAction(TimeLabelType type, QWidget *parent)
    :MyWidgetAction(parent)
{
    label_type = type;
    current_time = 0;
    total_time = 0;
    updateText();
}

TimeLabelAction::~TimeLabelAction(){
}

void TimeLabelAction::setCurrentTime(double t){
    current_time = t;
    updateText();
}

void TimeLabelAction::setTotalTime(double t){
    total_time = t;
    updateText();
}

void TimeLabelAction::updateText(){
    QString ct = Helper::formatTime(current_time);
    QString tt = Helper::formatTime(total_time);
    QString rt;
    if(total_time < 1)rt = "00:00:00";else rt = "-" + Helper::formatTime(total_time-current_time);

    switch(label_type){
    case CurrentTime:setText(ct);break;
    case TotalTime:setText(tt);break;
    case CurrentAndTotalTime:setText(ct+"/"+tt);break;
    case RemainingTime:setText(rt);break;
    }
}


void TimeLabelAction::setText(QString s){
    current_text = s;
    emit newtext(s);
}

QWidget *TimeLabelAction::createWidget(QWidget *parent){
    QLabel *time_label = new QLabel(parent);
    time_label->setObjectName("time_label");
    time_label->setAlignment(Qt::AlignVCenter|Qt::AlignHCenter);
    if(current_text.isEmpty())current_text="00:00:00/00:00:00";
    time_label->setText(current_text);
    connect(this,SIGNAL(newtext(QString)),
            time_label,SLOT(setText(QString)));
    return time_label;
}
