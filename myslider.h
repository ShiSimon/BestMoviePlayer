#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QSlider>

#define CODE_FOR_CLICK 1 //1=code copied from QSlider

class QTimer;

class MySlider : public QSlider
{
public:
    MySlider(QWidget * parent);
    ~MySlider();

protected:
    void mousePressEvent (QMouseEvent * event);
#if CODE_FOR_CLICK == 1
    inline int pick(const QPoint & pt)const;
    int pixelPosToRangeValue(int pos)const;
#endif
};

#endif // MYSLIDER_H
