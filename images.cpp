#include "images.h"
#include <QFile>
#include <QDebug>
#include <QResource>

QString Images::resourceFilename(){
    QString filename = QString::null;

    filename = "file";
    return filename;
}

void Images::setThemesPath(const QString &folder){

}

QString Images::file(const QString & name){
    QString icon_name;
    icon_name = ":/icons/"+name;
    icon_name += ".png";

    return icon_name;
}

QPixmap Images::icon(QString name, int size){
    QString icon_name = file(name);
    QPixmap p(icon_name);

    if(!p.isNull()){
        if(size != -1){
            p = resize(&p,size);
        }
    }
    //qDebug()<<icon_name;
    return p;
}

QPixmap Images::resize(QPixmap *p, int size){
    return QPixmap::fromImage((*p).toImage().scaled(size,size,Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
}


