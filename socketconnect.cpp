#include "socketconnect.h"
#include <QDebug>
#include <QHostAddress>
#include <QTextCodec>
#include <QHostInfo>
#include "tinyxml.h"
#include <stdio.h>
#include <list>
#include <QFile>
#include <string.h>
#include <string>
#include <utility>
#include <malloc.h>
#include <memory>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QByteArray>
#include <win32_compat.h>
#include <winsock2.h>
#include <time.h>
#include <fcntl.h>
#include "nfsc/libnfs.h"
#include "nfsc/libnfs-raw.h"
#include "nfsc/libnfs-raw-nfs.h"
#include "nfsc/libnfs-raw-mount.h"
#include "playlist.h"

SocketConnect::SocketConnect(QObject *parent) : QObject(parent)
{
    blockSize = 0;
    playlist = new PlayList();
    prikeybuffer = "";
    if(!TcpSocket.listen(QHostAddress::Any,5001))
    {
        qDebug()<<"listen error";
        qDebug()<<TcpSocket.errorString();
        return;
    }
    connect(&TcpSocket,SIGNAL(newConnection()),this,SLOT(acceptConnection()));
    if(!TcpSocket.isListening())
    {
        qDebug()<<"not listening";
    }
    qDebug()<<"SocketConnect::listening";
}

void SocketConnect::acceptConnection(){
    qDebug()<<"SocketConnect::accept";
    tcpclient = TcpSocket.nextPendingConnection();
    blockSize = 0;
    connect(tcpclient,SIGNAL(readyRead()),this,SLOT(readMessage()));
    connect(tcpclient,SIGNAL(disconnected()),tcpclient,SLOT(deleteLater()));
}

int SocketConnect::parse_CreateSpl(TiXmlDocument *test_doc)
{
    qDebug()<<"SocketConnect::parse_createspl";
    playlist->clear();
    TiXmlHandle xml_handle(test_doc);
    TiXmlNode *Command_text = xml_handle.FirstChild("command").FirstChild("command_text").FirstChild().Node();
    char *p = (char *)malloc(strlen(Command_text->Value()));
//    qDebug()<<Command_text->Value();
    strcpy(p,Command_text->Value());
    QString s = QString(QLatin1String(p));
    TiXmlDocument *Spl = new TiXmlDocument();
//    Spl->LoadFile(TIXML_ENCODING_UTF8);
    Spl->Parse(p,0,TIXML_ENCODING_UTF8);
    /*if(Spl->Parse(p) == NULL)
    {
        qDebug()<<"error";
        if(p) free(p);
        p = NULL;
        return -1;
    }*/
    if(p) free(p);
    p = NULL;
    qDebug()<<"SocketConnect::parse success!";
    TiXmlElement *Spl_Id = Spl->FirstChildElement()->FirstChildElement();
    TiXmlElement *Spl_Name = Spl_Id->NextSiblingElement()->NextSiblingElement()->NextSiblingElement()
            ->NextSiblingElement()->NextSiblingElement();
    TiXmlAttribute *Spl_Duration = Spl_Name->NextSiblingElement()->FirstAttribute();
    playlist->SetSplName(Spl_Name->GetText());
    QString duration_ = Spl_Duration->Value();
    playlist->SetSpltotaltime(duration_.toInt());
    TiXmlNode *pNode = Spl_Id->NextSiblingElement()->NextSiblingElement()->NextSiblingElement()
            ->NextSiblingElement()->NextSiblingElement()->NextSiblingElement()->FirstChildElement();
    //qDebug()<<pNode->Value();
    QString spl_id_ = Spl_Id->GetText();
    playlist->SetSplid(spl_id_);
    for(pNode;pNode;pNode=pNode->NextSiblingElement())
    {
        TiXmlElement *CompositionPlaylistId = pNode->ToElement();
        TiXmlAttribute *Dcp_url = CompositionPlaylistId->FirstAttribute();
        TiXmlAttribute *Cpl_Time = Dcp_url->Next();
        TiXmlAttribute *Title = CompositionPlaylistId->FirstAttribute()->Next()->Next(); 
        qDebug()<<"title = "<<Title->Name();
        TiXmlAttribute *KDM_url = Title->Next();
        QString cpl_time_ = Cpl_Time->Value();
        playlist->addFile(Dcp_url->Value(),KDM_url->Value(),Title->Name()
                          ,cpl_time_.toInt(),CompositionPlaylistId->GetText());
    }
    //emit createspl();
    return 0;
}

int SocketConnect::parse_SelectSpl(TiXmlDocument *test_doc)
{
    TiXmlHandle xml_handle(test_doc);
    TiXmlElement *Spl_uuid = xml_handle.FirstChild("command").FirstChild("spl_uuid").ToElement();
    if(playlist->Splid() != Spl_uuid->GetText())
    {
        qDebug()<<"spl error!";
        return -1;
    }
    return 0;
    emit selectspl();
}

void SocketConnect::make_response(const char *message,int error)
{
    response = new TiXmlDocument();
    response->LoadFile(TIXML_ENCODING_UTF8);
    TiXmlDeclaration *re_Dec = new TiXmlDeclaration("1.0","UTF-8","");
    response->LinkEndChild(re_Dec);
    TiXmlElement *re_stat = new TiXmlElement("response");
    if(error == 0)
    {
        re_stat->SetAttribute("status","OK");
        re_stat->SetAttribute("version","2");
        response->LinkEndChild(re_stat);
    }
    else if(error == -1)
    {
        re_stat->SetAttribute("status","ERROR");
        re_stat->SetAttribute("version","2");
        TiXmlElement *error = new TiXmlElement("error");
        error->SetValue(message);
        re_stat->LinkEndChild(error);
        response->LinkEndChild(re_stat);
    }
}

void SocketConnect::readMessage(){
    QDataStream in(tcpclient);
    int ret = 0;
    in.setByteOrder(QDataStream::BigEndian);
    if(blockSize == 0)
    {
        if(tcpclient->bytesAvailable() < sizeof(header)) return;
        for(int k = 0;k<16;k++)
        {
            in >> header[k];
            if(header[k] != start_header[k]) return;
        }
        if(tcpclient->bytesAvailable() < sizeof(quint32)) return;
        in.setByteOrder(QDataStream::LittleEndian);
        in >> blockSize;
        in.setByteOrder(QDataStream::BigEndian);
    }
    if(tcpclient->bytesAvailable() < blockSize)
    {
        return;
    }
    char *buffer = (char *)malloc(tcpclient->bytesAvailable());
    in.readRawData(buffer,blockSize);
    QString s = QString(QLatin1String(buffer));
    blockSize = 0;
    TiXmlDocument *test_doc = new TiXmlDocument();
    if(test_doc->Parse(s.toUtf8().data(),0,TIXML_ENCODING_UTF8) == NULL)
    {
        if(buffer) free(buffer);
        buffer = NULL;
    }
    if(buffer) free(buffer);
    buffer = NULL;
    TiXmlHandle xml_handle(test_doc);
    TiXmlAttribute *cmd_type =xml_handle.FirstChild("command").ToElement()->FirstAttribute()->Next();
    QString cmd_type_ = cmd_type->Value();
    if(cmd_type_.indexOf("CREATESPL") == 0)
    {
        if(ret = parse_CreateSpl(test_doc) < 0)
        {
            qDebug("SocketConnect::CREATESPL error ret = %d",ret);
            make_response("Can not create spl.",-1);
            send_response();
        }
        else
        {
            make_response("",0);
            send_response();
            emit createspl();
        }
    }
    else if(cmd_type_.indexOf("SELECTSPL") == 0)
    {
        if(ret = parse_SelectSpl(test_doc)<0)
        {
            qDebug("SocketConnect::SELECTSPL error ret = %d",ret);
            make_response("Can not select this spl.",-1);
            send_response();
        }
        else
        {
            //make_response("",0);
            emit selectspl();
        }
    }
    else if(cmd_type_.indexOf("PLAYSPL") == 0)
    {
        emit playspl();
    }
    else if(cmd_type_.indexOf("STOPSPL") == 0)
    {
        emit stopspl();
    }
    else if(cmd_type_.indexOf("GETPLAYBACKSTATUS") == 0)
    {
        emit get_status();
    }
    else if(cmd_type_.indexOf("GETPRIKEYINFO") == 0)
    {
        emit getprikeyinfo();
    }
    else if(cmd_type_.indexOf("SETPRIKEYINFO") == 0)
    {
        parse_setprikeyinfo(test_doc);
    }
    else if(cmd_type_.indexOf("GETSERVERDATETIME") == 0)
    {
        emit getservertime();
    }
    else if(cmd_type_.indexOf("SETSERVERDATETIME") == 0)
    {
        parse_setserverdate(test_doc);
    }
    test_doc->Clear();
}

void SocketConnect::send_response()
{
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    for(int i = 0;i < 16;i++)
    {
        out<<(quint8)start_header[i];
    }
    TiXmlPrinter printer;
    response->Accept(&printer);
    qint32 xml_size = printer.Size();
    out.setByteOrder(QDataStream::LittleEndian);
    out<<(quint32)xml_size;
    out.setByteOrder(QDataStream::BigEndian);
    out.writeRawData(printer.CStr(),xml_size);
    delete response;
    if(tcpclient->isWritable())
    {
        tcpclient->write(block);
    }
}

void SocketConnect::send_response(TiXmlDocument *status_response)
{
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    for(int i = 0;i < 16;i++)
    {
        out<<(quint8)start_header[i];
    }
    TiXmlPrinter printer;
    status_response->Accept(&printer);
    qint32 xml_size = printer.Size();
    qDebug()<<xml_size;
    out.setByteOrder(QDataStream::LittleEndian);
    out<<(quint32)xml_size;
    out.setByteOrder(QDataStream::BigEndian);
    out.writeRawData(printer.CStr(),xml_size);
    if(tcpclient->isWritable())
    {
        tcpclient->write(block);
    }
}

void SocketConnect::response_getstatus(int status, QString cpl_uuid, qint32 duration){
    qDebug("SocketConnect::response_getstatus");
    TiXmlDocument *response = new TiXmlDocument();
    response->LoadFile(TIXML_ENCODING_UTF8);
    TiXmlDeclaration *re_Dec = new TiXmlDeclaration("1.0","UTF-8","");
    response->LinkEndChild(re_Dec);
    TiXmlElement *re_stat = new TiXmlElement("response");
    re_stat->SetAttribute("status","OK");
    re_stat->SetAttribute("version","2");
    //response->LinkEndChild(re_stat);
    TiXmlElement *PlayBackMode = new TiXmlElement("playbackmode");
    TiXmlText *PlayBackMode_Value = new TiXmlText("2D");
    PlayBackMode->LinkEndChild(PlayBackMode_Value);
    re_stat->LinkEndChild(PlayBackMode);
    if(cpl_uuid != ""){
        qDebug("cpl_uuid is not empty");
        TiXmlElement *Status = new TiXmlElement("status");
        QString status_value;
        switch(status)
        {
        case 0:status_value = "READY";break;
        case 1:status_value = "PLAYING";break;
        case 3:status_value = "STOPPED";break;
        case 2:status_value = "PAUSED";break;
        }
        Status->SetAttribute("state",status_value.toStdString().c_str());
        re_stat->LinkEndChild(Status);
        TiXmlElement *Show_UUID = new TiXmlElement("show_uuid");
        TiXmlText *Show_UUID_Value = new TiXmlText(playlist->Splid().toLocal8Bit());
        Show_UUID->LinkEndChild(Show_UUID_Value);
        Status->LinkEndChild(Show_UUID);
        TiXmlElement *Show_Name = new TiXmlElement("show_name");
        TiXmlText *Show_Name_Value = new TiXmlText(playlist->SplName().toUtf8().constData());
//        TiXmlText *Show_Name_Value = new TiXmlText("test");
        Show_Name->LinkEndChild(Show_Name_Value);
        Status->LinkEndChild(Show_Name);
        TiXmlElement *Show_Postion = new TiXmlElement("show_postion");
        int num;
        for(int i=0; i < playlist->pl.count(); i++)
        {
            if(cpl_uuid == playlist->pl.at(i).cpluuid()){
                num = i;
            }
        }
        qint32 played_duration = duration;
        for(int i = 0; i < num;i++){
            played_duration += playlist->pl.at(i).totaltime();
        }
        Show_Postion->SetAttribute("total_duration",playlist->Spltotaltime());
        Show_Postion->SetAttribute("played_duration",played_duration);
        Status->LinkEndChild(Show_Postion);
        TiXmlElement *Cpl_UUID = new TiXmlElement("cpl_uuid");
        TiXmlText *Cpl_UUID_Value = new TiXmlText(cpl_uuid.toLocal8Bit());
        Cpl_UUID->LinkEndChild(Cpl_UUID_Value);
        Status->LinkEndChild(Cpl_UUID);
        TiXmlElement *Cpl_Name = new TiXmlElement("cpl_name");
        TiXmlText *Cpl_Name_Value = new TiXmlText(playlist->pl.at(num).title().toUtf8().data());
//        TiXmlText *Cpl_Name_Value = new TiXmlText("test");
        Cpl_Name->LinkEndChild(Cpl_Name_Value);
        Status->LinkEndChild(Cpl_Name);
        TiXmlElement *Cpl_Postion = new TiXmlElement("cpl_postion");
        Cpl_Postion->SetAttribute("total_duration",playlist->pl.at(num).totaltime());
        Cpl_Postion->SetAttribute("cpl_index","1");
        Cpl_Postion->SetAttribute("played_position",duration);
        Status->LinkEndChild(Cpl_Postion);
    }
    response->LinkEndChild(re_stat);
    send_response(response);
}

void SocketConnect::responseGetprikeyInfo(QString appid){
    QFileInfo f("platforms/test.enc");
    bool isexitprikey = false;
    if(f.exists() == true)
    {
        isexitprikey = true;
    }
    response = new TiXmlDocument();
    response->LoadFile(TIXML_ENCODING_UTF8);
    TiXmlDeclaration *re_Dec = new TiXmlDeclaration("1.0","UTF-8","");
    response->LinkEndChild(re_Dec);
    TiXmlElement *re_stat = new TiXmlElement("response");
    re_stat->SetAttribute("status","OK");
    re_stat->SetAttribute("version","2");
    TiXmlElement *isexit = new TiXmlElement("isexit");
    QString v;
    if(isexitprikey == false){
        v="false";
    }else{
        v="true";
    }
    TiXmlText *isexit_Value = new TiXmlText(v.toUtf8().constData());
    isexit->LinkEndChild(isexit_Value);
    re_stat->LinkEndChild(isexit);
    TiXmlElement *serial = new TiXmlElement("serial");
    TiXmlText *serial_Value = new TiXmlText("SMETPC1607130946");
    serial->LinkEndChild(serial_Value);
    re_stat->LinkEndChild(serial);
    response->LinkEndChild(re_stat);
    send_response();
}

void SocketConnect::responseGetserverTime(QString time){
    response = new TiXmlDocument();
    response->LoadFile(TIXML_ENCODING_UTF8);
    TiXmlDeclaration *re_Dec = new TiXmlDeclaration("1.0","UTF-8","");
    response->LinkEndChild(re_Dec);
    TiXmlElement *re_stat = new TiXmlElement("response");
    re_stat->SetAttribute("status","OK");
    re_stat->SetAttribute("version","2");
    TiXmlElement *isotime = new TiXmlElement("iso_date_time");
    TiXmlText *isotime_Value = new TiXmlText(time.toUtf8().data());
    isotime->LinkEndChild(isotime_Value);
    re_stat->LinkEndChild(isotime);
    response->LinkEndChild(re_stat);
    send_response();
}

void SocketConnect::parse_setprikeyinfo(TiXmlDocument *test_doc){
    qDebug("SocketConnect::parse_prikeyinfo");
    TiXmlHandle xml_handle(test_doc);
    TiXmlElement *prikey = xml_handle.FirstChild("command").FirstChildElement().Element();
    TiXmlAttribute *prikeylength = prikey->FirstAttribute();
    QString key_length = prikeylength->Value();
    QString prikeyValue = prikey->GetText();
    if(key_length.toInt() != prikeyValue.length()){
        qDebug("SocketConnect::set prikeyinfo error");
        make_response("Can not set private key.",-1);
        send_response();
    }
    else {
        emit setprikeyinfo(prikey->GetText());
    }
}

void SocketConnect::parse_setserverdate(TiXmlDocument *test_doc){
    qDebug("SocketConnect::parse_setserverdate");
    TiXmlHandle xml_handle(test_doc);
    TiXmlElement *servertime = xml_handle.FirstChild("command").FirstChildElement().Element();
    QString s = servertime->GetText();
    emit setserverdatetime(s);
}
