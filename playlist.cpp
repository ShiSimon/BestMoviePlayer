#include "playlist.h"

PlayList::PlayList(QObject *parent) : QObject(parent)
{
    clear();
}

void PlayList::clear(){
    splid = "";
    splname = "";
    spl_total_time = 0;
    pl.clear();
}

int PlayList::count(){
    return pl.count();
}

bool PlayList::isEmpty(){
    return pl.isEmpty();
}

QString PlayList::Splid(){
    return splid;
}

void PlayList::SetSplid(QString spl){
    splid = spl;
}

void PlayList::addFile(QString dcpurl,QString kdmurl,QString title,qint32 time,QString cpl_uuid){
    pl.append(PlayListItem(dcpurl,kdmurl,title,time,cpl_uuid));
}
