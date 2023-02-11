#include "DecWithFFmpegH264.h"
#include <QDebug>

namespace VCodec {
DecWithFFmpegH264::DecWithFFmpegH264()
    : m_avcodec_ctx_(nullptr)
    , m_device_ctx(nullptr)
    , is_init_(false)
    , m_hwDeviceType_(AV_HWDEVICE_TYPE_NONE){
    av_register_all();
}

DecWithFFmpegH264::~DecWithFFmpegH264() {

}

void DecWithFFmpegH264::release(){
    is_init_ = false;
    m_hwDeviceType_ = AV_HWDEVICE_TYPE_NONE;

    if(m_avcodec_ctx_){
        avcodec_free_context(&m_avcodec_ctx_);
        m_avcodec_ctx_ = nullptr;
    }

    if(m_device_ctx){
        av_buffer_unref(&m_device_ctx);
        m_device_ctx = nullptr;
    }

    if(m_avPacket){
        av_packet_free(&m_avPacket);
        m_avPacket = nullptr;
    }

    if(m_avFrame){
        av_frame_free(&m_avFrame);
        m_avFrame = nullptr;
    }

}

int DecWithFFmpegH264::open_codec() {
    AVCodecID codecId = AV_CODEC_ID_H264;
    if(mcontext_.config_.codecType == CODEC_TYPE_H265){
        codecId = AV_CODEC_ID_H265;
    }

    AVCodec *avcodec_ = avcodec_find_decoder(codecId);
    if(nullptr == avcodec_){
        return -1;
    }
    int i = 0;
    AVPixelFormat m_pixelformat_ =  AV_PIX_FMT_NONE;
    //查找到对应硬件类型解码后的数据格式
    for(;;i++){
        const AVCodecHWConfig *config = avcodec_get_hw_config(avcodec_, i);
        if((config->methods == AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) &&
            (config->device_type == AV_HWDEVICE_TYPE_QSV ||
            config->device_type == AV_HWDEVICE_TYPE_VAAPI ||
            config->device_type == AV_HWDEVICE_TYPE_DXVA2)) {
            m_hwDeviceType_ = config->device_type;
            m_pixelformat_ = config->pix_fmt;
            break;
        }
    }

    do{
        if(AV_HWDEVICE_TYPE_NONE == m_hwDeviceType_ ||
           AV_PIX_FMT_NONE == m_pixelformat_){
            break;
        }

        if(nullptr == (m_avcodec_ctx_ = avcodec_alloc_context3(avcodec_))){
            break;
        }

        //解码器格式赋值为硬解码
        m_avcodec_ctx_->pix_fmt = m_pixelformat_;
        m_avcodec_ctx_->opaque = this;
        //创建硬解码设备
        if(0 > (av_hwdevice_ctx_create(&m_device_ctx, m_hwDeviceType_, NULL, NULL, 0))){
            qDebug()<<"create hardwar device context failed!"<<endl;
            break;
        }

        m_avcodec_ctx_->hw_device_ctx = m_device_ctx;

        if(0 > avcodec_open2(m_avcodec_ctx_, avcodec_, NULL)){
            qDebug()<<"open avcodec failed!"<<endl;
            break;
        }
        if(nullptr == (m_avFrame = av_frame_alloc())){
            qDebug()<<"avframe alloc failed!"<<endl;
            break;
        }
        if(nullptr == (m_avPacket = av_packet_alloc())){
            qDebug()<<"avpacket alloc failed!"<<endl;
            break;
        }
        av_init_packet(m_avPacket);
        is_init_ = true;
        return 0;
    }while(false);

    m_pixelformat_ = AV_PIX_FMT_NONE;
    release();
    return 0;
}

int DecWithFFmpegH264::start() {
    return 0;
}

int DecWithFFmpegH264::exit() {
    return 0;
}

}
