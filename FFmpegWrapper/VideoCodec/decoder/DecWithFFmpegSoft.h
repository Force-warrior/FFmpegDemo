#ifndef DECWITHFFMPEGSOFT_H
#define DECWITHFFMPEGSOFT_H

#include "VideoDecBase.h"

namespace VCodec {
class DecWithFFmpegSoft : public VideoDecBase
{
public:
    DecWithFFmpegSoft();
    virtual ~DecWithFFmpegSoft();
public:
    virtual int open_codec() override;
    virtual int start() override;
    virtual int exit() override;
private:
    void release();
private:
    AVCodecContext* m_codec_ctx_;
    AVFrame *m_avFrame;
    AVPacket* m_avPacket;
    bool is_init = false;
};
}
#endif // DECWITHFFMPEGSOFT_H
