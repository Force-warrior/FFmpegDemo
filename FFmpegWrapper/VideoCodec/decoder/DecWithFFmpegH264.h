#ifndef DECWITHFFMPEGH264_H
#define DECWITHFFMPEGH264_H

#include "VideoDecBase.h"

namespace VCodec {
class DecWithFFmpegH264: public VideoDecBase
{
public:
    DecWithFFmpegH264();
    virtual ~DecWithFFmpegH264();
public:
    virtual int open_codec() override;
    virtual int start() override;
    virtual int exit() override;
private:
    void release();

private:
    bool is_init_;
    AVCodecContext *m_avcodec_ctx_;
    AVHWDeviceType m_hwDeviceType_;
    AVBufferRef *m_device_ctx;

    AVPacket* m_avPacket;
    AVFrame* m_avFrame;
};

}

#endif // DECWITHFFMPEGH264_H
