#include "DecWithFFmpegSoft.h"
#include <QDebug>

namespace VCodec {
DecWithFFmpegSoft::DecWithFFmpegSoft()
    : m_codec_ctx_(nullptr),
      m_avFrame(nullptr),
      m_avPacket(nullptr),
      is_init(false){

}

DecWithFFmpegSoft::~DecWithFFmpegSoft(){
    release();
}

void DecWithFFmpegSoft::release(){
    is_init = false;
    if(m_codec_ctx_){
        avcodec_free_context(&m_codec_ctx_);
        m_codec_ctx_ = nullptr;
    }
    if(m_avFrame){
        av_frame_free(&m_avFrame);
        m_avFrame = nullptr;
    }
    if(m_avPacket){
        av_packet_free(&m_avPacket);
        m_avPacket = nullptr;
    }
}

int DecWithFFmpegSoft::open_codec(){
    if(is_init)
        return 0;
    AVCodecID codec_id = AV_CODEC_ID_H264;
    if(mcontext_.config_.codecType == CODEC_TYPE_H265_SOFT){
        codec_id = AV_CODEC_ID_H265;
    }
    AVPixelFormat m_ePixelFmt = AV_PIX_FMT_YUV420P;
    do{
        AVCodec *m_codec_ = avcodec_find_decoder(codec_id);
        if(nullptr == m_codec_){
            qDebug()<<"find decoder failed!"<<endl;
            break;
        }
        if(nullptr == (m_codec_ctx_ = avcodec_alloc_context3(m_codec_))){
            qDebug()<<"create avcodec_context failed!"<<endl;
            break;
        }

        if(!avcodec_open2(m_codec_ctx_, m_codec_, NULL)){
            qDebug()<<"open avcodec failed!"<<endl;
            break;
        }

        //判断读取码流的状态 ,可能以截断的方式来读取
        if(m_codec_->capabilities & AV_CODEC_CAP_TRUNCATED){
            m_codec_ctx_->flags != AV_CODEC_CAP_TRUNCATED;
        }

        if(nullptr ==(m_avFrame = av_frame_alloc())){
            qDebug()<<"avframe alloc failed!"<<endl;
            break;
        }
        if(nullptr ==(m_avPacket = av_packet_alloc())){
            qDebug()<<"avpacket alloc failed!"<<endl;
            break;
        }
        av_init_packet(m_avPacket);
        is_init = true;
        return 0;
    }while(false);

    release();
    return -1;
}

int DecWithFFmpegSoft::start(){
    if(!is_init)
        return 0;
    m_avPacket->data = mcontext_.inputdata_;
    m_avPacket->size = mcontext_.ndatasize_;
    m_avPacket->pts = mcontext_.timestamp_;
    int got_picture_ptr = 0;
    if(avcodec_decode_video2(m_codec_ctx_, m_avFrame, &got_picture_ptr, m_avPacket)){
        av_packet_free(&m_avPacket);
        return -1;
    }
    if(got_picture_ptr){
        VideoFrame outframe;
        outframe.fmt = FMT_I420;
        outframe.width = m_avFrame->width;// m_ptCodecCtx->width;
        outframe.height = m_avFrame->height;// m_ptCodecCtx->height;
        outframe.pts = m_avFrame->pts;
        outframe.data[0] = m_avFrame->data[0];
        outframe.linesize[0] = m_avFrame->linesize[0];
        outframe.data[1] = m_avFrame->data[1];
        outframe.linesize[1] = m_avFrame->linesize[1];
        outframe.data[2] = m_avFrame->data[2];
        outframe.linesize[2] = m_avFrame->linesize[2];

        mcontext_.handler_->onFrameDecoded(&outframe);
    }

    av_packet_free(&m_avPacket);
    return 0;
}

int DecWithFFmpegSoft::exit(){
    release();
    return 0;
}

}
