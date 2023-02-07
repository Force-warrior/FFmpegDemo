#ifndef IFFMPEGDELEGATE_H
#define IFFMPEGDELEGATE_H

#include "ffmpeg_def.h"

class IVideoCaptureHandler{
public:
   virtual ~IVideoCaptureHandler(){}
   virtual void onCaptureVideoFrame(VideoFrame& frame) = 0;
};

struct VideoCaptureOutPut{
    IVideoCaptureHandler* eventhandler;
    PIXEL_FORMAT type;
    int width;
    int height;

    VideoCaptureOutPut()
        : eventhandler(nullptr)
        , type(FMT_I420)
        , width(0)
        , height(0){}
};

#endif // IFFMPEGDELEGATE_H
