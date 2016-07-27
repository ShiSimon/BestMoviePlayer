#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QWidget>
#include <QList>
#include <QStringList>
#include <QObject>

class PlayListItem{

public:
    PlayListItem(){_dcp_url = "";_kdm_url = "";_title = "",_total_time = 0;_cpl_uuid = "";}
    PlayListItem(QString dcp_url,QString kdm_url,QString title,qint32 total_time,QString cpl_uuid){
        _dcp_url = dcp_url;
        _kdm_url = kdm_url;
        _title = title;
        _total_time = total_time;
        _cpl_uuid = cpl_uuid;
    };

    void SetDcpurl(QString dcp_url){_dcp_url = dcp_url;}
    void SetKdmurl(QString kdm_url){_kdm_url = kdm_url;}
    void SetTitle(QString title){_title = title;}
    void SetTotalTime(qint32 time){_total_time = time;}
    void SetCpluuid(QString uuid){_cpl_uuid = uuid;}

    QString dcpurl()const{return _dcp_url;}
    QString kdmurl()const{return _kdm_url;}
    QString title()const{return _title;}
    QString cpluuid()const{return _cpl_uuid;}
    qint32 totaltime()const{return _total_time;}

private:
    QString _dcp_url,_kdm_url;
    QString _title,_cpl_uuid;
    qint32 _total_time;
};


class PlayList : public QObject
{
    Q_OBJECT
public:
    PlayList(QObject *parent = 0);

    void clear();
    int count();
    bool isEmpty();
    QString Splid();
    void SetSplid(QString spl);
    void SetSplName(QString name){splname = name;}
    void SetSpltotaltime(qint32 time){spl_total_time = time;}
    void addFile(QString dcpurl,QString kdmurl,QString title = "",qint32 time = 0,QString cpl_uuid = "");

    QString SplName(){return splname;}
    qint32 Spltotaltime(){return spl_total_time;}

public:
    typedef QList <PlayListItem> PlayItemList;
    PlayItemList pl;
    QString splname;
protected:
    QString splid;
    qint32 spl_total_time;
};

#endif // PLAYLIST_H
