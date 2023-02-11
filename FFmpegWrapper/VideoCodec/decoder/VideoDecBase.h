#ifndef VIDEODECIMPL_H
#define VIDEODECIMPL_H

#include "IVideoCodec.h"
#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/samplefmt.h"
#include "libavutil/timestamp.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"

#include "libavdevice/avdevice.h"
#ifdef __cplusplus
}
#endif

namespace VCodec {
    struct VDecContext{
        IVideoDecHandler* handler_;
        DecConfig config_;
        uint8_t * inputdata_;
        int32_t ndatasize_;
        int64_t timestamp_;
    };


    class VideoDecBase : public IVideoDec
    {
    public:
        VideoDecBase();
        virtual ~VideoDecBase();
        virtual bool Initialize(IVideoDecHandler* handler, DecConfig& config) override;
        virtual int decode(uint8_t *data, int32_t len, int64_t timestamp);
        virtual int reconfig(DecConfig& config) override;
    protected:
        virtual int open_codec() = 0;
        virtual int start() = 0;
        virtual int exit() = 0;
    protected:
        VDecContext mcontext_;
    };
}

#endif // VIDEODECIMPL_H
