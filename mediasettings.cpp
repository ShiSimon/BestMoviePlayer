#include "mediasettings.h"
#include "config.h"

MediaSettings::MediaSettings()
{
    reset();
}

MediaSettings::~MediaSettings(){

}

void MediaSettings::reset()
{
    mute = false;
    volume = Init_Vol;
    fullscreen = false;
    aspect_ratio_id = Aspect169;
    win_width  = 400;
    win_height = 300;
    audio_use_channels = ChStereo;
    audio_delay = 0;
    stereo_mode = 0;
    factor = 100;
    zoom_factor = 1.0;
}

double MediaSettings::win_aspect(){
    return (double)win_width/win_height;
}

double MediaSettings::aspectToNum(Aspect aspect){
    double asp;

    switch(aspect){
    case MediaSettings::Aspect43:asp = (double)4/3;break;
    case MediaSettings::Aspect169:asp = (double)16/9;break;
    case MediaSettings::Aspect149:asp = (double)13/9;break;
    case MediaSettings::Aspect1610:asp = (double)16/10;break;
    case MediaSettings::Aspect54:asp = (double)5/4;break;
    case MediaSettings::Aspect235:asp = 2.35;break;
    case MediaSettings::Aspect11:asp = 1;break;
    case MediaSettings::Aspect32:asp = (double)3/2;break;
    case MediaSettings::Aspect1410:asp = (double)14/10;break;
    case MediaSettings::Aspect118:asp = (double)11/8;break;
    case MediaSettings::AspectAuto:asp = win_aspect();break;
    default:asp = win_aspect();
    }

    return asp;
}

