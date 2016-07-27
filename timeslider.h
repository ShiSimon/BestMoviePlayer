#ifndef TIMESLIDER_H
#define TIMESLIDER_H

#include "myslider.h"

class TimeSlider : public MySlider
{
    Q_OBJECT
public:
    TimeSlider(QWidget *parent);
    ~TimeSlider();

public slots:
    virtual void setPos(int); //Don't use setValue!
    virtual int pos();
    virtual void setDuration(double t){total_time = t;};
    virtual double duration(){return total_time;};

    void setDragDelay(int);
    int dragDelay();

signals:
    void posChanged(int);
    void draggingPos(int);
    void delayedDraggingPos(int);
    void wheelUp();
    void wheelDown();

protected slots:
    void stopUpdate();
    void resumeUpdate();
    void mouseReleased();
    void valueChanged_slot(int);
    void checkDragging(int);
    void sendDelayedPos();

protected:
    virtual void wheelEvent(QWheelEvent *e);
    virtual bool event(QEvent *event);

private:
    bool dont_update;
    int position;
    double total_time;

    int last_pos_to_send;
    QTimer *timer;

};

#endif // TIMESLIDER_H
