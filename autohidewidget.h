#ifndef AUTOHIDEWIDGET_H
#define AUTOHIDEWIDGET_H

#include <QWidget>

class QTimer;

class AutoHideWidget : public QWidget
{
    Q_OBJECT
public:
    enum Activation {Anywhere = 1,Bottom = 2};
    AutoHideWidget(QWidget *parent = 0);
    ~AutoHideWidget();

    void setInternalWidget(QWidget *w);
    QWidget *internalWidget(){return internal_widget;};

public slots:
    void show();
    void activate();
    void deactivate();
    void setAutoHide(bool b);
    void setAnimated(bool b){use_animation = b;};
    void setMargin(int margin){spacing = margin;};
    void setPercWidth(int s){perc_width = s;}
    void setActivationArea(Activation m){activation_area = m;}
    void setHideDelay(int ms);
public:
    bool isActive(){return turned_on;}
    bool autoHide(){return auto_hide;}
    int margin(){return spacing;}
    int percWidth(){return perc_width;};
    Activation activationArea(){return activation_area;}
    int hideDelay();

protected:
    bool eventFilter(QObject * obj,QEvent *event);

private slots:
    void checkUnderMouse();
    void showAnimated();

private:
    void installFiliter(QObject *o);
    void resizeAndMove();
    bool visiblePopups();

private:
    bool turned_on;
    bool auto_hide;
    bool use_animation;
    int spacing;
    int perc_width;
    Activation activation_area;
    QWidget *internal_widget;
    QTimer *timer;
};


#endif // AUTOHIDEWIDGET_H
