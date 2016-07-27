#ifndef MYWIDGETACTION_H
#define MYWIDGETACTION_H

#include <QWidgetAction>
#include "timeslider.h"

class QStyle;

class MyWidgetAction : public QWidgetAction
{
    Q_OBJECT

public:
    MyWidgetAction(QWidget *parent);
    ~MyWidgetAction();

    void setCustomStyle(QStyle *style){custom_style = style;};
    QStyle *customStyle(){return custom_style;};

    void setStyleSheet(QString style){custom_stylesheet = style;};
    QString styleSheet(){return custom_stylesheet;};

public slots:
    virtual void enable();
    virtual void disable();

protected:
    virtual void propagate_enabled(bool);

protected:
    QStyle *custom_style;
    QString custom_stylesheet;
};

class TimeSliderAction:public MyWidgetAction
{
    Q_OBJECT

public:
    TimeSliderAction(QWidget * parent);
    ~TimeSliderAction();

public slots:
    virtual void setPos(int);
    virtual int pos();
    virtual void setDuration(double);
    virtual double duration(){return total_time;};

    void setDragDelay(int);
    int dragDelay();

private:
    int drag_delay;
    double total_time;

signals:
    void posChanged(int value);
    void draggingPos(int value);
    void delayDraggingPos(int);
    void wheelUp();
    void wheelDown();
protected:
    virtual QWidget * createWidget(QWidget * parent);
};

class VolumeSliderAction:public MyWidgetAction
{
    Q_OBJECT

public:
    VolumeSliderAction( QWidget * parent );
    ~VolumeSliderAction();

    void setFixedSize(QSize size) { fixed_size = size; };
    QSize fixedSize() { return fixed_size; };

    void setTickPosition(QSlider::TickPosition position);
    QSlider::TickPosition tickPosition() { return tick_position; };

public slots:
    virtual void setValue(int);
    virtual int value();

signals:
    void valueChanged(int value);

protected:
    virtual QWidget * createWidget ( QWidget * parent );

private:
    QSize fixed_size;
    QSlider::TickPosition tick_position;
};

class TimeLabelAction:public MyWidgetAction
{
    Q_OBJECT

public:
    enum TimeLabelType {CurrentTime = 0,TotalTime = 1,CurrentAndTotalTime = 2,RemainingTime = 3};

    TimeLabelAction(TimeLabelType type,QWidget* parent);
    ~TimeLabelAction();

    virtual QString text() {return current_text;};

public slots:
    virtual void setText(QString s);
    virtual void setCurrentTime(double);
    virtual void setTotalTime(double);

signals:
    void newtext(QString s);

protected:
    virtual QWidget *createWidget(QWidget * parent);
    virtual void updateText();

private:
    QString current_text;
    double current_time,total_time;
    TimeLabelType label_type;
};

#endif // MYWIDGETACTION_H
