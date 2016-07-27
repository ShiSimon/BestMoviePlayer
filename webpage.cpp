#include "webpage.h"
//#include <QtWebSockets/QtWebSockets>
#include <QWebFrame>
#include <QWebView>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtNetwork>
#include <QAction>
#include <QWebPage>
#include <QMovie>
#include <QLabel>
#include <QHostInfo>

#include "mywebmessage.h"
#include "images.h"
#include "global.h"
#include "preference.h"

using namespace Global;

QString g_appid;

WebPage::WebPage(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ischecked = false;
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    //获取本地appid和ip信息
    getmac();
    g_appid = appid;
    page = new QWebView(this);
    page->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    toolwidget = new QWidget();
    toolwidget->setObjectName("webtoolwidget");
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    home_button = new QPushButton("go-home",toolwidget);
    home_button->setText(tr(""));
    home_button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);
    home_button->setIcon(Images::icon("go-home"));
    home_button->setIconSize(QSize(26,26));
    back_button = new QPushButton("back",toolwidget);
    back_button->setText(tr(""));
    back_button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);
    back_button->setIcon(Images::icon("back"));
    back_button->setIconSize(QSize(26,26));
    forward_button = new QPushButton("forward",toolwidget);
    forward_button->setText(tr(""));
    forward_button->setIcon(Images::icon("forward"));
    forward_button->setIconSize(QSize(26,26));
    forward_button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);
    refresh_button = new QPushButton("refresh",toolwidget);
    refresh_button->setText(tr(""));
    refresh_button->setIconSize(QSize(26,26));
    refresh_button->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);
    refresh_button->setIcon(Images::icon("refresh"));
    loading = new QLabel(toolwidget);
    loading->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Minimum);
    QMovie *movie=new QMovie("D:/QT Test/MyDemonPlayer/icons/loading.gif");
    loading->setMovie(movie);
    movie->start();
    connect(home_button,SIGNAL(clicked(bool)),this,SLOT(handlehome()));
    connect(back_button,SIGNAL(clicked(bool)),this,SLOT(handleback()));
    connect(forward_button,SIGNAL(clicked(bool)),this,SLOT(handleforward()));
    connect(refresh_button,SIGNAL(clicked(bool)),this,SLOT(handelrefresh()));
    urlbar = new QProgressBar();
    urlbar->setSizePolicy(QSizePolicy::Expanding, urlbar->sizePolicy().verticalPolicy());
    urlbar->setTextVisible(false);
    urlbar->setRange(0,100);
    layout->addWidget(home_button);
    layout->addWidget(back_button);
    layout->addWidget(forward_button);
    layout->addWidget(refresh_button);
    layout->addWidget(urlbar);
    layout->addWidget(loading);
    toolwidget->setLayout(layout);
    QVBoxLayout *fulllayout = new QVBoxLayout;
    fulllayout->setSpacing(0);
    fulllayout->setMargin(0);
    fulllayout->addWidget(toolwidget);
    fulllayout->addWidget(page);
    setLayout(fulllayout);
    QWebSettings * settings = page->settings();
    QWebSettings::globalSettings();
    settings->setAttribute(QWebSettings::JavaEnabled, true);
    settings->setAttribute(QWebSettings::JavascriptEnabled, true);
    settings->setAttribute(QWebSettings::JavascriptCanOpenWindows, true);
    settings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    settings->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    settings->setAttribute(QWebSettings::SpatialNavigationEnabled, true);
    settings->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
    settings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, true);
    settings->setAttribute(QWebSettings::AutoLoadImages, true);
    page->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    page->settings()->setAttribute(QWebSettings::PluginsEnabled,true);
    page->load(QUrl(pref->mainpage));
//    page->load(QUrl("http://www.baidu.com"));
    connect(page, SIGNAL(loadProgress(int)),
            this, SLOT(setProgress(int)));
    connect(page, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished()));
    page->pageAction(QWebPage::Reload)->setIconText(tr("刷新"));
    page->pageAction(QWebPage::Reload)->setText(tr("刷新"));
    page->page()->action(QWebPage::InspectElement)->setVisible(false);

    m_webObj = new MyWebMessage(this);
    connect(page->page()->mainFrame(),SIGNAL(javaScriptWindowObjectCleared()),
            this,SLOT(populateJavaScriptWindowObject()));

//    QString strVal = QString("callfromqt(\"SMETPC07130946\",\"10.1.3.72\");");
//    page->page()->mainFrame()->evaluateJavaScript(strVal);
    page->setFocus();

    //connect(page->page(), SIGNAL(loadingUrl(QUrl)),
//            this, SIGNAL(urlChanged(QUrl)));
    //page->show();
}

WebPage::~WebPage(){

}

void WebPage::setProgress(int progress){
    m_progress = progress;
    urlbar->setValue(m_progress);
    loading->show();
}

void WebPage::loadFinished(){
    ischecked = true;
    if (100 != m_progress) {

    }
    m_progress = 0;
    urlbar->setValue(m_progress);
    loading->hide();
    if(page->url() == (QUrl)pref->mainpage){
        qDebug()<<ipaddress;
        QString strVal = QString("callfromqt(\"SMETPC1607130946\",\"%1\");").arg(ipaddress);
        page->page()->mainFrame()->evaluateJavaScript(strVal);
    }
}

void WebPage::handlehome(){
    page->load(QUrl(pref->mainpage));
}

void WebPage::handleback(){
    page->back();
}

void WebPage::handleforward(){
    page->forward();
}

void WebPage::handelrefresh(){
    page->reload();
}

void WebPage::populateJavaScriptWindowObject(){
    qDebug()<<"populateJavaScriptWindowObject";
    page->page()->mainFrame()->addToJavaScriptWindowObject(QString("mywebkit"),m_webObj);
}

void WebPage::getmac(){
    QStringList mac_list;
    QStringList ip_list;
    QString strMac;
    QString ip;
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    foreach(QNetworkInterface interfaceItem, ifaces)
    {
        if(interfaceItem.flags().testFlag(QNetworkInterface::IsUp)
                &&interfaceItem.flags().testFlag(QNetworkInterface::IsRunning)
                &&interfaceItem.flags().testFlag(QNetworkInterface::CanBroadcast)
                &&interfaceItem.flags().testFlag(QNetworkInterface::CanMulticast)
                &&!interfaceItem.flags().testFlag(QNetworkInterface::IsLoopBack)
                &&interfaceItem.hardwareAddress()!="00:50:56:C0:00:01"
                &&interfaceItem.hardwareAddress()!="00:50:56:C0:00:08")
        {
            QList<QNetworkAddressEntry> addressEntryList=interfaceItem.addressEntries();
            foreach(QNetworkAddressEntry addressEntryItem, addressEntryList)
            {
                if(addressEntryItem.ip().protocol()==QAbstractSocket::IPv4Protocol)
                {
                    ip = addressEntryItem.ip().toString();
                    ip_list.append(ip);
                    strMac = interfaceItem.hardwareAddress();
                    mac_list.append(strMac);
                }
            }
        }
    }
    for(int i = 0; i < ip_list.count(); i++){
        if(ip_list.value(i).startsWith(pref->ip_domain)){
                QString mac = mac_list.value(i).remove(":");
                appid = "SMETPC" + mac;
                ipaddress = ip_list.value(i);
        }
    }
}
