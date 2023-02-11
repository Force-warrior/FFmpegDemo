#ifndef I_VIDEOCODEC_H_
#define I_VIDEOCODEC_H_
#include "ffmpeg_def.h"

namespace VCodec {
    class IVideoEncHandler{
    public:
        virtual ~IVideoEncHandler(){}
        virtual int onFrameEncoded(VideoFrame *frame) { return 0; }
    };

    class IVideoDecHandler{
    public:
       virtual ~IVideoDecHandler(){}
        virtual int onFrameDecoded(VideoFrame *frame) { return 0; }
    };


    class IVideoEnc{
    public:
        virtual  bool Initialize(IVideoEncHandler* handler, EncConfig& config) = 0;
        virtual int reconfig(EncConfig& config) = 0;
        virtual int encode(VideoFrame *inframe, bool forceIFrmae = 0) = 0;
    };

    class IVideoDec{
    public:
        virtual bool Initialize(IVideoDecHandler* handler, DecConfig& config) = 0;
        virtual int reconfig(DecConfig& config) = 0;
    };

    IVideoEnc* CreateVideoEnc();
    void ReleaseVideoEnc(IVideoEnc*enc);
    IVideoDec* CreateVideoDec();
    void ReleaseVideoDec(IVideoDec*dec);
}


#endif
