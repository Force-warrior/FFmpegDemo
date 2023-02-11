#ifndef VIDEOENCIMPL_H
#define VIDEOENCIMPL_H

#include "IVideoCodec.h"
namespace VCodec {
    class VideoEncBase : public IVideoEnc
    {
    public:
        VideoEncBase();
        virtual ~VideoEncBase();
        virtual bool Initialize(IVideoEncHandler* handler, EncConfig& config) override;
        virtual int reconfig(EncConfig& config) override;
        virtual int encode(VideoFrame *inframe, bool forceIFrmae = 0) override;
    protected:
        virtual bool init() = 0;
    private:
        IVideoEncHandler* mhandler_;
        EncConfig mconfig_;
    };
}
#endif // VIDEOENCIMPL_H
