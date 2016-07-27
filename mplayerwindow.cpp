#include "mplayerwindow.h"

#include <QLabel>
#include <QTimer>
#include <QCursor>
#include <QEvent>
#include <QLayout>
#include <QPixmap>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QDebug>

#include "images.h"

Screen::Screen(QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f )
    , check_mouse_timer(0)
    , mouse_last_position(QPoint(0,0))
    , autohide_cursor(false)
    , autohide_interval(0)
{
    setMouseTracking(true);
    setFocusPolicy( Qt::NoFocus );
    setMinimumSize( QSize(0,0) );

    check_mouse_timer = new QTimer(this);
    connect( check_mouse_timer, SIGNAL(timeout()), this, SLOT(checkMousePos()) );

    setAutoHideInterval(1000);
    setAutoHideCursor(false);
}

Screen::~Screen() {
}

void Screen::setAutoHideCursor(bool b) {
    qDebug("Screen::setAutoHideCursor: %d", b);

    autohide_cursor = b;
    if (autohide_cursor) {
        check_mouse_timer->setInterval(autohide_interval);
        check_mouse_timer->start();
    } else {
        check_mouse_timer->stop();
    }
}

void Screen::checkMousePos() {
    //qDebug("Screen::checkMousePos");
    if (!autohide_cursor) {
        setCursor(QCursor(Qt::ArrowCursor));
        return;
    }
    QPoint pos = mapFromGlobal(QCursor::pos());
    //qDebug("Screen::checkMousePos: x: %d, y: %d", pos.x(), pos.y());
    if (mouse_last_position != pos) {
        setCursor(QCursor(Qt::ArrowCursor));
    } else {
        setCursor(QCursor(Qt::BlankCursor));
    }
    mouse_last_position = pos;
}

void Screen::mouseMoveEvent( QMouseEvent * e ) {
    //qDebug("Screen::mouseMoveEvent");
    emit mouseMoved(e->pos());

    if (cursor().shape() != Qt::ArrowCursor) {
        //qDebug(" showing mouse cursor" );
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

void Screen::playingStarted() {
    qDebug("Screen::playingStarted");
    setAutoHideCursor(true);
}

void Screen::playingStopped() {
    qDebug("Screen::playingStopped");
    setAutoHideCursor(false);
}

/* ---------------------------------------------------------------------- */

MplayerLayer::MplayerLayer(QWidget* parent, Qt::WindowFlags f)
    : Screen(parent, f)
    , repaint_background(false)
    , playing(false)
{
#ifndef Q_OS_WIN
#if QT_VERSION < 0x050000
    setAttribute(Qt::WA_OpaquePaintEvent);
#if QT_VERSION >= 0x040400
    setAttribute(Qt::WA_NativeWindow);
#endif
    setAttribute(Qt::WA_PaintUnclipped);
    //setAttribute(Qt::WA_PaintOnScreen);
#endif
#endif
    //setAttribute(Qt::WA_NativeWindow);
}

MplayerLayer::~MplayerLayer() {
}

void MplayerLayer::setRepaintBackground(bool b) {
    qDebug("MplayerLayer::setRepaintBackground: %d", b);
    repaint_background = b;
}

void MplayerLayer::paintEvent( QPaintEvent * e ) {
    //qDebug("MplayerLayer::paintEvent: repaint_background: %d", repaint_background);
    if (repaint_background || !playing) {
        //qDebug("MplayerLayer::paintEvent: painting");
        QPainter painter(this);
        painter.eraseRect( e->rect() );
        //painter.fillRect( e->rect(), QColor(255,0,0) );
    }
}

void MplayerLayer::playingStarted() {
    qDebug("MplayerLayer::playingStarted");
    repaint();
    playing = true;

#ifndef Q_OS_WIN
    setAttribute(Qt::WA_PaintOnScreen);
#endif

    //setAttribute(Qt::WA_PaintOnScreen);
    Screen::playingStarted();
}

void MplayerLayer::playingStopped() {
    qDebug("MplayerLayer::playingStopped");
    playing = false;

#ifndef Q_OS_WIN
    setAttribute(Qt::WA_PaintOnScreen, false);
#endif

    repaint();
    Screen::playingStopped();
}

/* ---------------------------------------------------------------------- */

MplayerWindow::MplayerWindow(QWidget* parent, Qt::WindowFlags f)
    : Screen(parent, f)
    , video_width(0)
    , video_height(0)
    , aspect((double) 4/3)
    , monitoraspect(0)
    , mplayerlayer(0)
    , logo(0)
    , offset_x(0)
    , offset_y(0)
    , zoom_factor(1.0)
    , orig_x(0)
    , orig_y(0)
    , orig_width(0)
    , orig_height(0)
    , allow_video_movement(false)
    , delay_left_click(false)
    , left_click_timer(0)
    , double_clicked(false)
    , corner_widget(0)
    , drag_state(NOT_DRAGGING)
    , start_drag(QPoint(0,0))
    , mouse_drag_tracking(false)
{
    mplayerlayer = new MplayerLayer(this);
    mplayerlayer->setObjectName("mplayerlayer");

    logo = new QLabel( mplayerlayer );
    logo->setObjectName("mplayerwindowlogo");

    // Set colors
#ifdef CHANGE_WIDGET_COLOR
    setAutoFillBackground(true);
    ColorUtils::setBackgroundColor( this, QColor(0,0,0) );
    mplayerlayer->setAutoFillBackground(true);
    logo->setAutoFillBackground(true);
    ColorUtils::setBackgroundColor( logo, QColor(0,0,0) );
#else
    setStyleSheet("MplayerWindow { background-color: black;}");
    mplayerlayer->setStyleSheet("background-color: black;");
#endif

    QVBoxLayout * mplayerlayerLayout = new QVBoxLayout( mplayerlayer );
    mplayerlayerLayout->addWidget( logo, 0, Qt::AlignHCenter | Qt::AlignVCenter );

    setSizePolicy( QSizePolicy::Expanding , QSizePolicy::Expanding );
    setFocusPolicy( Qt::StrongFocus );

    grabGesture(Qt::TapGesture);

    installEventFilter(this);
    mplayerlayer->installEventFilter(this);

    left_click_timer = new QTimer(this);
    left_click_timer->setSingleShot(true);
    left_click_timer->setInterval(qApp->doubleClickInterval()+10);
    connect(left_click_timer, SIGNAL(timeout()), this, SIGNAL(leftClicked()));

    retranslateStrings();
}

MplayerWindow::~MplayerWindow() {
}

void MplayerWindow::setCornerWidget(QWidget * w) {
    corner_widget = w;

    QHBoxLayout * blayout = new QHBoxLayout;
    blayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));
    blayout->addWidget(corner_widget);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));
    layout->addLayout(blayout);
}

void MplayerWindow::retranslateStrings() {
    //qDebug("MplayerWindow::retranslateStrings");
    logo->setPixmap(Images::icon("background"));
    logo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void MplayerWindow::setLogoVisible( bool b) {
    if (corner_widget) {
        corner_widget->setVisible(b);
    }
    logo->setVisible(b);
}

void MplayerWindow::setResolution( int w, int h)
{
    video_width = w;
    video_height = h;

    //mplayerlayer->move(1,1);
    updateVideoWindow();
}

void MplayerWindow::resizeEvent( QResizeEvent * /* e */)
{
    /*qDebug("MplayerWindow::resizeEvent: %d, %d",
       e->size().width(), e->size().height() );*/

#if !DELAYED_RESIZE
    offset_x = 0;
    offset_y = 0;

    qDebug("resize factor = %f",zoom_factor);
    updateVideoWindow();
    setZoom(zoom_factor);
#else
    resize_timer->start();
#endif
}

void MplayerWindow::setMonitorAspect(double asp) {
    monitoraspect = asp;
}

void MplayerWindow::setAspect( double asp) {
    aspect = asp;
    if (monitoraspect!=0) {
        aspect = aspect / monitoraspect * DesktopInfo::desktop_aspectRatio(this);
    }
    updateVideoWindow();
}


void MplayerWindow::updateVideoWindow()
{
    int w_width = size().width();
    int w_height = size().height();

    int w = w_width;
    int h = w_height;
    int x = 0;
    int y = 0;

    if (aspect != 0) {
        int pos1_w = w_width;
        int pos1_h = w_width / aspect + 0.5;

        int pos2_h = w_height;
        int pos2_w = w_height * aspect + 0.5;

        //qDebug("pos_w:%d,pos_h:%d",w_width,w_height);
        //qDebug("pos1_w:%d,pos1_h:%d",pos1_w,pos1_h);
        //qDebug("pos2_w:%d,pos2_h:%d",pos2_w,pos2_h);

        if ((pos1_h+30) <= w_height) {
            //qDebug("Pos1!");
            w = pos1_w;
            h = pos1_h;

            y = (w_height - h) /2;
        } else {
            //qDebug("Pos2!");
            w = pos2_w;
            h = pos2_h;

            x = (w_width - w) /2;
        }
    }

    if(x<=0){x=-x;}
    if(y<=0){y=-y;}

    //qDebug("MplayerWindow x=%d y=%d",x,y);
    //qDebug("MplayerWindow w=%d h=%d",w,h);
    mplayerlayer->move(x,y);
    mplayerlayer->resize(w, h);

    orig_x = x;
    orig_y = y;
    orig_width = w;
    orig_height = h;
}


void MplayerWindow::mouseReleaseEvent( QMouseEvent * e) {
    qDebug( "MplayerWindow::mouseReleaseEvent" );

    if (e->button() == Qt::LeftButton) {
        e->accept();
        if (delay_left_click) {
            if (!double_clicked) left_click_timer->start();
            double_clicked = false;
        } else {
            emit leftClicked();
        }
    }
    else
        if (e->button() == Qt::MidButton) {
            e->accept();
            emit middleClicked();
        }
        else
            if (e->button() == Qt::XButton1) {
                e->accept();
                emit xbutton1Clicked();
            }
            else
                if (e->button() == Qt::XButton2) {
                    e->accept();
                    emit xbutton2Clicked();
                }
                else
                    if (e->button() == Qt::RightButton) {
                        e->accept();
                        //emit rightButtonReleased( e->globalPos() );
                        emit rightClicked();
                    }
                    else {
                        e->ignore();
                    }
}

void MplayerWindow::mouseDoubleClickEvent( QMouseEvent * e ) {
    if (e->button() == Qt::LeftButton) {
        e->accept();
        if (delay_left_click) {
            left_click_timer->stop();
            double_clicked = true;
        }
        emit doubleClicked();
    } else {
        e->ignore();
    }
}

void MplayerWindow::wheelEvent( QWheelEvent * e ) {
    qDebug("MplayerWindow::wheelEvent: delta: %d", e->delta());
    e->accept();

    if (e->orientation() == Qt::Vertical) {
        if (e->delta() >= 0)
            emit wheelUp();
        else
            emit wheelDown();
    } else {
        qDebug("MplayerWindow::wheelEvent: horizontal event received, doing nothing");
    }
}

bool MplayerWindow::eventFilter( QObject * object, QEvent * event ) {
    if (!mouse_drag_tracking)
        return false;

    QEvent::Type type = event->type();
    if (type != QEvent::MouseButtonPress
            && type != QEvent::MouseButtonRelease
            && type != QEvent::MouseMove)
        return false;

    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (!mouseEvent)
        return false;

    if (mouseEvent->modifiers() != Qt::NoModifier) {
        drag_state = NOT_DRAGGING;
        return false;
    }

    if (type == QEvent::MouseButtonPress) {
        if (mouseEvent->button() != Qt::LeftButton) {
            drag_state = NOT_DRAGGING;
            return false;
        }

        drag_state = START_DRAGGING;
        start_drag = mouseEvent->globalPos();
        // Don't filter, so others can have a look at it too
        return false;
    }

    if (type == QEvent::MouseButtonRelease) {
        if (drag_state != DRAGGING || mouseEvent->button() != Qt::LeftButton) {
            drag_state = NOT_DRAGGING;
            return false;
        }

        // Stop dragging and eat event
        drag_state = NOT_DRAGGING;
        event->accept();
        return true;
    }

    // type == QEvent::MouseMove
    if (drag_state == NOT_DRAGGING)
        return false;

    // buttons() note the s
    if (mouseEvent->buttons() != Qt::LeftButton) {
        drag_state = NOT_DRAGGING;
        return false;
    }

    QPoint pos = mouseEvent->globalPos();
    QPoint diff = pos - start_drag;
    if (drag_state == START_DRAGGING) {
        // Don't start dragging before moving at least DRAG_THRESHOLD pixels
        if (abs(diff.x()) < DRAG_THRESHOLD && abs(diff.y()) < DRAG_THRESHOLD)
            return false;

        drag_state = DRAGGING;
    }

    emit mouseMovedDiff(diff);
    start_drag = pos;
    event->accept();
    return true;
}

QSize MplayerWindow::sizeHint() const {
    //qDebug("MplayerWindow::sizeHint");
    return QSize( video_width, video_height );
}

QSize MplayerWindow::minimumSizeHint () const {
    return QSize(0,0);
}

void MplayerWindow::setOffsetX( int d) {
    offset_x = d;
    mplayerlayer->move( orig_x + offset_x, mplayerlayer->y() );
}

int MplayerWindow::offsetX() { return offset_x; }

void MplayerWindow::setOffsetY( int d) {
    offset_y = d;
    mplayerlayer->move( mplayerlayer->x(), orig_y + offset_y );
}

int MplayerWindow::offsetY() { return offset_y; }

void MplayerWindow::setZoom( double d) {
    zoom_factor = d;
    offset_x = 0;
    offset_y = 0;

    int x = orig_x;
    int y = orig_y;
    int w = orig_width;
    int h = orig_height;

    if (zoom_factor != 1.0) {
        w = w * zoom_factor;
        h = h * zoom_factor;

        // Center
        x = (width() - w) / 2;
        y = (height() -h) / 2;
    }

    mplayerlayer->move(x,y);
    mplayerlayer->resize(w,h);
}

double MplayerWindow::zoom() { return zoom_factor; }

void MplayerWindow::moveLayer( int offset_x, int offset_y ) {
    int x = mplayerlayer->x();
    int y = mplayerlayer->y();

    mplayerlayer->move( x + offset_x, y + offset_y );
}

void MplayerWindow::moveLeft() {
    if ((allow_video_movement) || (mplayerlayer->x()+mplayerlayer->width() > width() ))
        moveLayer( -16, 0 );
}

void MplayerWindow::moveRight() {
    if ((allow_video_movement) || ( mplayerlayer->x() < 0 ))
        moveLayer( +16, 0 );
}

void MplayerWindow::moveUp() {
    if ((allow_video_movement) || (mplayerlayer->y()+mplayerlayer->height() > height() ))
        moveLayer( 0, -16 );
}

void MplayerWindow::moveDown() {
    if ((allow_video_movement) || ( mplayerlayer->y() < 0 ))
        moveLayer( 0, +16 );
}

void MplayerWindow::incZoom() {
    setZoom( zoom_factor + ZOOM_STEP );
}

void MplayerWindow::decZoom() {
    double zoom = zoom_factor - ZOOM_STEP;
    if (zoom < ZOOM_MIN) zoom = ZOOM_MIN;
    setZoom( zoom );
}

// Language change stuff
void MplayerWindow::changeEvent(QEvent *e) {
    if (e->type() == QEvent::LanguageChange) {
        retranslateStrings();
    } else {
        Screen::changeEvent(e);
    }
}
