#ifndef MPLAYERWINDOW_H
#define MPLAYERWINDOW_H

#include "desktopinfo.h"

#include <QObject>
#include <QWidget>
#include <QSize>
#include <QPoint>
#include <QResizeEvent>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>

class QWidget;
class QLabel;
class QKeyEvent;
class QTimer;

#define ZOOM_STEP 0.05
#define ZOOM_MIN 0.5

#define DELAYED_RESIZE 0

// Number of pixels the window has to be dragged at least before dragging starts
#define DRAG_THRESHOLD 4

enum TDragState {NOT_DRAGGING, START_DRAGGING, DRAGGING};

//! Screen is a widget that hides the mouse cursor after some seconds if not moved.

class Screen : public QWidget
{
    Q_OBJECT

public:
    Screen(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~Screen();

    void setAutoHideCursor(bool b);
    bool autoHideCursor() { return autohide_cursor; };

    void setAutoHideInterval(int milliseconds) { autohide_interval = milliseconds; };
    int autoHideInterval() { return autohide_interval; };

public slots:
    //! Should be called when a file has started.
    virtual void playingStarted();

    //! Should be called when a file has stopped.
    virtual void playingStopped();

signals:
    void mouseMoved(QPoint);

protected:
    virtual void mouseMoveEvent( QMouseEvent * e );

protected slots:
    virtual void checkMousePos();

private:
    QTimer * check_mouse_timer;
    QPoint mouse_last_position;
    bool autohide_cursor;
    int autohide_interval;
};

//! MplayerLayer can be instructed to not delete the background.

class MplayerLayer : public Screen
{
    Q_OBJECT

public:
    MplayerLayer(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~MplayerLayer();


    //! If b is true, the background of the widget will be repainted as usual.
    /*! Otherwise the background will not repainted when a video is playing. */
    void setRepaintBackground(bool b);

    //! Return true if repainting the background is allowed.
    bool repaintBackground() { return repaint_background; };


public slots:
    //! Should be called when a file has started.
    /*! It's needed to know if the background has to be cleared or not. */
    virtual void playingStarted();
    //! Should be called when a file has stopped.
    virtual void playingStopped();

protected:
    virtual void paintEvent ( QPaintEvent * e );

private:

    bool repaint_background;
    bool playing;
};

class MplayerWindow:public Screen
{
    Q_OBJECT
public:
    MplayerWindow(QWidget * parent = 0,Qt::WindowFlags f = 0);
    ~MplayerWindow();

    MplayerLayer *videoLayer(){return mplayerlayer;};

    void setResolution(int w,int h);
    void setAspect( double asp);
    void setMonitorAspect(double asp);
    void updateVideoWindow();

    void setOffsetX( int );
    int offsetX();

    void setOffsetY( int );
    int offsetY();

    void setZoom( double );
    double zoom();

    void allowVideoMovement(bool b) { allow_video_movement = b; };
    bool isVideoMovementAllowed() { return allow_video_movement; };

    void delayLeftClick(bool b) { delay_left_click = b; };
    bool isLeftClickDelayed() { return delay_left_click; };

    virtual QSize sizeHint () const;
    virtual QSize minimumSizeHint() const;

    virtual bool eventFilter(QObject *, QEvent *);


    void setCornerWidget(QWidget * w);
    QWidget * cornerWidget() { return corner_widget; };

public slots:
    void setLogoVisible(bool b);
    void showLogo() { setLogoVisible(true); };
    void hideLogo() { setLogoVisible(false); };

    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    void incZoom();
    void decZoom();

    void activateMouseDragTracking(bool active) { mouse_drag_tracking = active; }

protected:
    virtual void retranslateStrings();
    virtual void changeEvent ( QEvent * event ) ;

    virtual void resizeEvent( QResizeEvent * e);
    virtual void mouseReleaseEvent( QMouseEvent * e);
    virtual void mouseDoubleClickEvent( QMouseEvent * e );
    virtual void wheelEvent( QWheelEvent * e );
    void moveLayer( int offset_x, int offset_y );

signals:
    //void rightButtonReleased( QPoint p );
    void doubleClicked();
    void leftClicked();
    void rightClicked();
    void middleClicked();
    void xbutton1Clicked(); // first X button
    void xbutton2Clicked(); // second X button
    void keyPressed(QKeyEvent * e);
    void wheelUp();
    void wheelDown();
    void mouseMovedDiff(QPoint);

protected:
    int video_width, video_height;
    double aspect;
    double monitoraspect;

    MplayerLayer * mplayerlayer;
    QLabel * logo;

    // Zoom and moving
    int offset_x, offset_y;
    double zoom_factor;

    // Original pos and dimensions of the mplayerlayer
    // before zooming or moving
    int orig_x, orig_y;
    int orig_width, orig_height;

    bool allow_video_movement;

    // Delay left click event
    bool delay_left_click;
    QTimer * left_click_timer;
    bool double_clicked;
    QWidget * corner_widget;

private:
    TDragState drag_state;
    QPoint start_drag;
    bool mouse_drag_tracking;
};

#endif // MPLAYERWINDOW_H
