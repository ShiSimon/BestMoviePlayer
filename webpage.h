#ifndef WEBPAGE_H
#define WEBPAGE_H

//#include <QtWebKit/QtWebKit>
#include <QtWebKitWidgets/QWebView>
#include <QWidget>
#include <QUrl>
#include <QPushButton>
#include <QProgressBar>

class QAction;
class QLabel;
class MyWebMessage;

class WebPage : public QWidget
{
    Q_OBJECT
public:
    WebPage(QWidget *parent = 0);
    ~WebPage();

    QWebView *page;

    bool ischecked;

public:
    void getmac();

public slots:
    void handlehome();
    void handleback();
    void handleforward();
    void handelrefresh();

protected slots:
    void loadFinished();
   // void urlChanged(const QUrl&);
    void setProgress(int);
    void populateJavaScriptWindowObject();

private:
    QWidget *toolwidget;
    QPushButton *home_button;
    QPushButton *back_button;
    QPushButton *forward_button;
    QPushButton *refresh_button;
    QProgressBar *urlbar;
    QLabel *loading;
    int m_progress;
    QUrl url() const;
    QString appid,ipaddress;
    MyWebMessage *m_webObj;
};

#endif // WEBPAGE_H
