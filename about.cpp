#include "about.h"
#include "ui_about.h"

#include <QFile>
#include <QUrl>
#include <QDesktopServices>
#include <QDebug>
#include <QtNetwork/QtNetwork>
#include <QList>

#include "images.h"

#define MainPage "http://www.bestmovie.com.cn/"

About::About(QWidget *parent,Qt::WindowFlags f) :
    QDialog(parent,f),
    ui(new Ui::About)
{
    ui->setupUi(this);
    setWindowIcon(Images::icon("logo",64));
    QString Mac;
    Mac = getappID();
    ui->logo->setPixmap(QPixmap(":/icons/logo.png").scaledToHeight(64,Qt::SmoothTransformation));
    ui->info->setText(
                "<b>DcpPlayer</b>"
                "<br>Created By SMET</br>"
                "<br><b>"+tr("Links:")+"</b><br>"+
                tr("Offical website:")+" "+link(MainPage)+
                "<br><b>"+tr("AppID:")+"</b><br>" + Mac);
    connect(ui->info,SIGNAL(anchorClicked(const QUrl &)),this,SLOT(openlink(const QUrl &)));
    connect(ui->OkButton,SIGNAL(clicked(bool)),this,SLOT(Exitwidget()));
}

About::~About()
{
    delete ui;
}

QString About::link(const QString &url, QString name){
    if(name.isEmpty()) name = url;
    return QString("<a href=\"" + url + "\">" + name + "</a>");
}

void About::openlink(const QUrl & link){
    //qDebug("About::openLink:%s",link.toStdString());
    qDebug("About::openLink");
    QDesktopServices::openUrl(link);
}


void About::Exitwidget(){
    close();
}

QString About::getMac(){
    QStringList mac_list;
    QString strMac;
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    for(int i = 0; i<ifaces.count();i++)
    {
        QNetworkInterface iface = ifaces.at(0);
        if(!(iface.flags().testFlag(QNetworkInterface::IsLoopBack)))
        {
            if(!(iface.humanReadableName().contains("VMare",Qt::CaseInsensitive)))
            {
                strMac = iface.hardwareAddress();
                mac_list.append(strMac);
                //qDebug()<<"strMac="<<strMac;
            }
        }
    }
    QString mac = "0413" + mac_list.value(0).remove(":") +
            mac_list.value(1).remove(":") + "0831";
    return mac;
}
QString About::getappID(){
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
    QString mac = mac_list.value(0).remove(":");
    mac = "SMETPC" + mac;
    return mac;
}
