#ifndef MEDIASETTINGS_H
#define MEDIASETTINGS_H

#include <QString>
#include <QSize>
#include "config.h"

class QSettings;

class MediaSettings
{
public:
    enum Aspect{AspectAuto = 1,Aspect43 = 2,Aspect54 = 3,Aspect149 = 4,
               Aspect169 = 5,Aspect1610 = 6,Aspect235 = 7,Aspect11 = 8,
               Aspect32 = 9,Aspect1410 = 10,Aspect118 = 11};
    enum AudioChannels{ChDefault = 0,ChStereo = 2,ChSurround = 4,ChFull51 = 6,
                      ChFull61 = 7};
    enum StereoMode{Stereo = 0,Left = 1,Right = 2,Mono = 3,Reverse = 4};
    MediaSettings();
    virtual ~MediaSettings();
    virtual void reset();

    int volume;
    int mute;
    bool fullscreen;
    int win_width;
    int win_height;
    int aspect_ratio_id;
    double win_aspect();
    double aspectToNum(Aspect aspect);
    int audio_use_channels;
    int audio_delay;
    int stereo_mode;
    int factor;
    double zoom_factor;
};

#endif // MEDIASETTINGS_H
