#ifndef DCPDATA_H
#define DCPDATA_H

#include <QString>
#include "kdm.h"
#include "tinyxml.h"

class KDM;

class DcpData
{
public:
    DcpData();
    ~DcpData();

    void reset();
    void set_cpl_url(QString s);
    void set_dcp_url(QString s);

    bool is_Dcp_Dir(QString path);
    void parseLocalDir();
    void parse_local_assetmap();
    void getKdmInfo();

    int parse_nfs_dcp(QString dcppath,QString kdmpath);
    int parse_nfs_dcpdir(QString dcp_url);
    int parse_nfs_cpl(TiXmlDocument *cpl_doc);
    int parse_nfs_assetmap(TiXmlDocument *asset_doc);

    QString dcp_title;
    double duration;
    double current_sec;

    //Resolution of the video
    int video_width;
    int video_height;
    double video_aspect;

    QString cpl_url;
    QString dcp_url;
    QString video_url;
    QString audio_url;
    QString kdm_url;
    QString cpl_uuid;
    QString assetmap_url;
    QString keyid;
    QString key;
    bool isenc;
    KDM kdm_info;

};

#endif // DCPDATA_H
