#include "dcpdata.h"
#include <cmath>
#include <QDir>
#include <QApplication>
#include <QDebug>
#include "tinyxml.h"
#include "config.h"
#include <stdio.h>
#include <string>
#include <utility>
#include <malloc.h>
#include <memory>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <win32_compat.h>
#include <winsock2.h>
#include <fcntl.h>
#include "tinyxml.h"
#include "nfsc/libnfs.h"
#include "nfsc/libnfs-raw.h"
#include "nfsc/libnfs-raw-nfs.h"
#include "nfsc/libnfs-raw-mount.h"

DcpData::DcpData()
{
    reset();
}

DcpData::~DcpData(){
}

void DcpData::set_cpl_url(QString s)
{
    cpl_url = s;
}

void DcpData::set_dcp_url(QString s)
{
    dcp_url = s;
}

void DcpData::reset(){
    dcp_title = "";
    duration = 0;
    video_width = 0;
    video_height = 0;
    cpl_url = "";
    video_url = "";
    audio_url = "";
    kdm_url = "";
    video_aspect = (double)16/9;
    dcp_url = "";
    assetmap_url = "";
    isenc = true;
    key="";
    keyid="";
    current_sec = 0;
}

bool DcpData::is_Dcp_Dir(QString path){
    qDebug()<<"DcpData::is_Dcp_dir";
    QDir dcp_path(path);
    QFileInfoList list = dcp_path.entryInfoList();
    int file_count = dcp_path.count();
    int must_file_num=0;
    if(file_count < 0 ){return false;}
    for(int i = 0;i < file_count; i++){
        QFileInfo fileinfo = list.at(i);
//        qDebug()<<i<<fileinfo.fileName();
        if((fileinfo.fileName()==".")|(fileinfo.fileName()=="..")){
            continue;
        }
        if(fileinfo.fileName().endsWith(".mxf"))
        {
            must_file_num ++;
        }
        if(fileinfo.fileName().contains("CPL")|fileinfo.fileName().contains("cpl"))
        {
            cpl_url = fileinfo.absoluteFilePath();
            must_file_num++;
        }
        if(fileinfo.fileName().startsWith("KDM")|fileinfo.fileName().startsWith("kdm"))
        {
            kdm_url = fileinfo.absoluteFilePath();
            qDebug()<<"kdm_url = "<<kdm_url;
            isenc = true;
        }
        if(fileinfo.fileName().startsWith("ASSETMAP")|fileinfo.fileName().startsWith("assetmap"))
        {
            assetmap_url = fileinfo.absoluteFilePath();
            must_file_num++;
        }
    }
    if(must_file_num < 3){return false;}
    dcp_url=path;
    return true;
}

void DcpData::parseLocalDir(){
    qDebug("DcpData::parseLocalDir");
    TiXmlDocument *cpl_doc = new TiXmlDocument();
    cpl_doc->LoadFile(cpl_url.toLocal8Bit().data());
    TiXmlHandle doc_handle(cpl_doc);
    TiXmlElement *issuer = doc_handle.FirstChildElement().ChildElement("Issuer",0).Element();
    QString is = issuer->GetText();
    qDebug()<<is;
    if(is == "Dev Center of SMET"){
        qDebug("The Media is MPEG-2+PCM");
        parse_local_assetmap();
        return;
    }
    TiXmlElement *cpl_id_real = doc_handle.FirstChildElement().FirstChildElement().Element();
    qDebug()<<cpl_id_real->GetText();
    cpl_uuid = cpl_id_real->GetText();
    TiXmlElement *reel_type = doc_handle.FirstChildElement().ChildElement("ReelList",0)
            .FirstChildElement().ChildElement("AssetList",0).FirstChildElement().Element();
    TiXmlElement *reel_type2 = reel_type->NextSiblingElement();
    if(strcmp(reel_type->Value(),"MainPicture")==0)
    {
        TiXmlElement *video_element = reel_type->FirstChildElement()->NextSiblingElement();
        TiXmlElement *key_id = video_element->NextSiblingElement()->NextSiblingElement()->NextSiblingElement()
                ->NextSiblingElement()->NextSiblingElement();
        video_url = video_element->GetText();
        keyid = key_id->GetText();
        QString s = key_id->Value();
        if(s != ("KeyId")){
            isenc = false;
        }
    }
    if(strcmp(reel_type2->Value(),"MainSound")==0)
    {
        TiXmlElement *audio_element = reel_type2->FirstChildElement()->NextSiblingElement();
        audio_url = audio_element->GetText();
    }
    qDebug()<<video_url;
    if(!video_url.endsWith(".mxf")){
        video_url = "";
        audio_url = "";
        qDebug("The Media is H264+PCM");
        parse_local_assetmap();
    }
    qDebug("The Media is H264+AC3");
}

void DcpData::parse_local_assetmap(){
    qDebug("DcpData::parse_local_assetmap");
    TiXmlDocument *asset_doc = new TiXmlDocument();
    asset_doc->LoadFile(assetmap_url.toLocal8Bit().data());
    TiXmlHandle doc_handle(asset_doc);
    TiXmlElement *path1 = doc_handle.FirstChildElement().ChildElement("AssetList",0).ChildElement("Asset",0)
            .ChildElement("ChunkList",0).FirstChildElement().ChildElement("Path",0).Element();
    QString s;
    s = path1->GetText();
    if(s.contains("audio") | s.startsWith("wav")){
        audio_url = s;
    }else if(s.contains("video") | s.startsWith("avc")){
        video_url = s;
    }
    TiXmlElement *path2 = doc_handle.FirstChildElement().ChildElement("AssetList",0).ChildElement("Asset",1)
            .ChildElement("ChunkList",0).FirstChildElement().ChildElement("Path",0).Element();
    s = path2->GetText();
    if(s.contains("audio") | s.startsWith("wav")){
        audio_url = s;
    }else if(s.contains("video") | s.startsWith("avc")){
        video_url = s;
    }
}

void DcpData::getKdmInfo(){
    kdm_info.Open(kdm_url.toLocal8Bit(),1); //local
    QString strfile;
    strfile = QCoreApplication::applicationDirPath();
    strfile = strfile + "/" + PrikeyPath;
    const char * pri_path = strfile.toLocal8Bit().constData();
    kdm_info.Dec(pri_path);
    for(int j = 0; j<16;j++)
    {
        QString strTmp;
        strTmp.sprintf("%02X",kdm_info.GetDecKeyList().begin()->aes_key_[j]);
        key.append(strTmp);
    }
    kdm_info.Close();
}

int DcpData::parse_nfs_dcp(QString dcppath, QString kdmpath){
    qDebug()<<"DcpData::parse_nfs_dcp";
    QString dcp_url_p = dcppath;
    QString kdm_url_p = kdmpath;
    dcp_url_p.remove(":");
    dcp_url_p = "nfs://" + dcp_url_p;
    kdm_url_p.remove(":");
    kdm_url_p = "nfs://" + kdm_url_p;
    QString k;
    if(kdm_info.Open(kdm_url_p.toLocal8Bit().constData(),2) != 0)
    {
        return -1;
    }
    QString strfile;
    strfile = QCoreApplication::applicationDirPath();
    strfile = strfile + "/" + PrikeyPath;
    const char * pri_path = strfile.toLocal8Bit().constData();
    if(kdm_info.Dec(pri_path) != 0){
        kdm_info.Close();
        return -2;
    }
    if(parse_nfs_dcpdir(dcp_url_p)==0)
    {
        for(int j = 0; j<16;j++)
        {
            QString strTmp;
            strTmp.sprintf("%02X",kdm_info.GetDecKeyList().begin()->aes_key_[j]);
            k.append(strTmp);
        }
    }else{return -3;}
    key = k;
    qDebug()<<"time is:"<<kdm_info.nv_after<<" "<<kdm_info.nv_before;
    kdm_info.Close();
    return 0;
}

int DcpData::parse_nfs_dcpdir(QString dcp_url)
{
    qDebug("DcpData::parse_nfs_dcpdir");
    struct nfs_context *nfs;
    struct nfsfh *nfsfh;
    struct nfs_url *url;
    struct nfs_stat_64 st;
    struct nfsdirent *nfsdirent;
    struct nfsdir *nfsdir;
    nfs = nfs_init_context();
    if (nfs == NULL)
    {
        fprintf(stderr, "failed to init context\n");
        goto finished;
    }
    url = nfs_parse_url_dir(nfs,dcp_url.toLocal8Bit());
    if (url == NULL)
    {
        fprintf(stderr, "%s\n", nfs_get_error(nfs));
        goto finished;
    }
    if (nfs_mount(nfs,url->server,url->path) != 0)
    {
        fprintf(stderr, "Failed to mount nfs share : %s\n",nfs_get_error(nfs));
        goto finished;
    }
    nfs_opendir(nfs,"/", &nfsdir);
    while((nfsdirent = nfs_readdir(nfs, nfsdir)) != NULL)
    {
        if (!strcmp(nfsdirent->name, ".") || !strcmp(nfsdirent->name, ".."))
        {continue;}
//        qDebug()<<"names="<<nfsdirent->name;
        if((strncmp(nfsdirent->name,"cpl_",4) == 0) ||(strncmp(nfsdirent->name,"CPL_",4) == 0))
        {
            char *buffer=(char *)malloc(1024*1024);
            if (nfs_open(nfs,nfsdirent->name,O_RDONLY,&nfsfh) != 0)
            {
                fprintf(stderr, "Failed to open file %s: %s\n",url->file,nfs_get_error(nfs));
                goto finished;
            }
            nfs_fstat64(nfs,nfsfh,&st);
            int readchunk;
            readchunk = nfs_get_readmax(nfs);
            int max_len;
            if(st.nfs_size < readchunk)
            {
                max_len = st.nfs_size;
            }
            int num;
            num = nfs_read(nfs,nfsfh,max_len,buffer);
            if(num < 0)
            {
                fprintf(stderr, "Failed to read file %s: %s\n",url->file,nfs_get_error(nfs));
                goto finished;
            }
            TiXmlDocument *cpl_doc = new TiXmlDocument;
            if(cpl_doc->Parse(buffer) == NULL)
            {
                qDebug()<<"error!";
                if(buffer) free(buffer);
                buffer = NULL;
            }
            if(buffer) free(buffer);
            buffer =  NULL;
            if(parse_nfs_cpl(cpl_doc) == 0)
            {
//                break;
            }
            else
            {
                goto finished;
            }
        }
        if((strncmp(nfsdirent->name,"assetmap",8) == 0) ||(strncmp(nfsdirent->name,"ASSETMAP",8) == 0))
        {
//            qDebug()<<nfsdirent->name;
            char *buffer=(char *)malloc(1024*1024);
            if (nfs_open(nfs,nfsdirent->name,O_RDONLY,&nfsfh) != 0)
            {
                fprintf(stderr, "Failed to open file %s: %s\n",url->file,nfs_get_error(nfs));
                goto finished;
            }
            nfs_fstat64(nfs,nfsfh,&st);
            int readchunk;
            readchunk = nfs_get_readmax(nfs);
            int max_len;
            if(st.nfs_size < readchunk)
            {
                max_len = st.nfs_size;
            }
            int num;
            num = nfs_read(nfs,nfsfh,max_len,buffer);
            if(num < 0)
            {
                fprintf(stderr, "Failed to read file %s: %s\n",url->file,nfs_get_error(nfs));
                goto finished;
            }
            TiXmlDocument *asset_doc = new TiXmlDocument;
            if(asset_doc->Parse(buffer) == NULL)
            {
                qDebug()<<"error!";
                if(buffer) free(buffer);
                buffer = NULL;
            }
            if(buffer) free(buffer);
            buffer =  NULL;
            if(parse_nfs_assetmap(asset_doc) == 0)
            {
//                break;
            }
            else
            {
                goto finished;
            }
        }
    }
    return 0;

finished:
    nfs_destroy_url(url);
    if (nfs != NULL) {
        nfs_destroy_context(nfs);
    }
    return -1;
}

int DcpData::parse_nfs_cpl(TiXmlDocument *cpl_doc)
{
    qDebug("DcpData::parse_nfs_cpl");
    QString cpl_uuid_s;
    QString cpl_key_id;
    QString s;
    char uuid_buffer[45];
    char key_buffer[45];
    TiXmlHandle doc_handle(cpl_doc);
    TiXmlElement *cpl_id_real = doc_handle.FirstChildElement().FirstChildElement().Element();
    kdm_info.GetCplUuid(uuid_buffer);
    cpl_uuid_s.append(uuid_buffer);
    cpl_uuid = cpl_id_real->GetText();
    if(!cpl_uuid_s.contains(cpl_id_real->GetText()))
    {
        TiXmlElement *reel_type = doc_handle.FirstChildElement().ChildElement("ReelList",0)
                .FirstChildElement().ChildElement("AssetList",0).FirstChildElement().Element();
        TiXmlElement *reel_type2 = reel_type->NextSiblingElement();
//        qDebug()<<reel_type->Value()<<reel_type2->Value();
        if(strcmp(reel_type->Value(),"MainPicture")==0)
        {
            TiXmlElement *video_element = reel_type->FirstChildElement()->NextSiblingElement();
            TiXmlElement *key_id = video_element->NextSiblingElement()->NextSiblingElement()->NextSiblingElement()
                    ->NextSiblingElement()->NextSiblingElement();
            kdm_info.GetMsgUuid(key_buffer);
            cpl_key_id.append(key_buffer);
            keyid = key_id->GetText();
//            qDebug()<<video_element->GetText();
//            qDebug()<<key_id->GetText();
            if(!cpl_key_id.contains(key_id->GetText()))
            {
                s = video_element->GetText();
                if(s.endsWith(".mxf")){
                video_url = video_element->GetText();
                qDebug()<<"DcpData::parse_nfs_cpl:video_url="<<video_url;}
            }
            else{return -1;}
        }
        if(strcmp(reel_type2->Value(),"MainSound")==0)
        {
            TiXmlElement *audio_element = reel_type2->FirstChildElement()->NextSiblingElement();
            s = audio_element->GetText();
            if(s.endsWith(".mxf")){
            audio_url = audio_element->GetText();
            }
//            qDebug()<<audio_element->GetText();
        }
    }
    return 0;
}

int DcpData::parse_nfs_assetmap(TiXmlDocument *asset_doc){
    qDebug("DcpData::parse_nfs_assetmap");
    TiXmlHandle doc_handle(asset_doc);
    TiXmlElement *uuid = doc_handle.FirstChildElement().FirstChildElement().Element();
    TiXmlElement *diff = uuid->NextSiblingElement();
    QString s = diff->Value();
    if(s != "AnnotationText")
    {
        qDebug("DcpData::parse_nfs_assetmap:is_smetDcp");
        return 0;
    }
    TiXmlElement *type1 = doc_handle.FirstChildElement().ChildElement("AssetList",0).FirstChildElement()
            .ChildElement("AnnotationText",0).Element();
    s = type1->GetText();
    if(s.endsWith("1 picture")){
        TiXmlElement *name1 = type1->NextSiblingElement()->FirstChildElement()->FirstChildElement();
        video_url = name1->GetText();
    }
    TiXmlElement *type2 = doc_handle.FirstChildElement().ChildElement("AssetList",0).ChildElement("Asset",1)
            .ChildElement("AnnotationText",0).Element();
    s = type2->GetText();

    if(s.endsWith("2 pcm")){
        TiXmlElement *name2 = type2->NextSiblingElement()->FirstChildElement()->FirstChildElement();
        audio_url = name2->GetText();
    }
    return 0;
}
