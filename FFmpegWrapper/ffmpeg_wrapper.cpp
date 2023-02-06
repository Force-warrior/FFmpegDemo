#include "ffmpeg_wrapper.h"

#include <thread>
#include <memory>

#include <QDebug>

static inline char* err2str(int eno){
    static char buf[1024];
    return av_make_error_string(buf, size_t(sizeof(buf)), eno);
}

class FFmpegWrapper::Impl: public enable_shared_from_this<Impl>{
public:
    struct CaptureThreadData{
        CaptureThreadData(std::shared_ptr<Impl> data): thiz_(data){
        }
        const std::shared_ptr<Impl> thiz_;
    };
public:
    Impl()
        :isinit_(false){

    }
    ~Impl(){
        isinit_ = false;
    }
public:
    bool Init(InitParam& param){
        if(isinit_)
            return true;
        memcpy(&init_param_, &param, sizeof (param));
        avdevice_register_all();
        isinit_ = true;
        return true;
    }

    void unInit(){
        isinit_ = false;
        isCaptureThreadExit = false;
    }

    bool start(){
        do{
            //创建设备上下文
            if(nullptr == (mFormatCtx_ = avformat_alloc_context())){
                qDebug() << "create AVFormatContext failed." << endl;
                break;
            }

            AVDictionary* options = nullptr;
            AVInputFormat *iformat = nullptr;
            //判断是否是摄像头
            if(CAPTURE_DEVICE == init_param_.in_capture_type){
                if (nullptr == (iformat = av_find_input_format("dshow"))) {
                    qDebug() << "create dshow failed." << endl;
                     break;
                }
            }

            char resolution[32];
            sprintf(resolution, "%dx%d", init_param_.in_video_width, init_param_.in_video_height);
            av_dict_set(&options, "video_size", resolution, 0);
            av_dict_set(&options, "rw_timeout", "3000", 0);

            qDebug()<<"open "<<init_param_.in_path_file_name_<<endl;
            if (avformat_open_input(&mFormatCtx_, init_param_.in_path_file_name_, iformat, &options) < 0) {
                qDebug()<<"open camera "<<init_param_.in_path_file_name_<<"failed"<<endl;
                break;
            }

            if ((avformat_find_stream_info(mFormatCtx_, NULL)) < 0) {
                qDebug()<<"Couldn't find video stream information."<<endl;
                break;
            }

            for(int i = 0; i < mFormatCtx_->nb_streams; i++){
                if(mFormatCtx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
                    nVideoIndex = i;
                    break;
                }
            }
            if(-1 == nVideoIndex) break;
            //查找编码器
            AVStream *stream = mFormatCtx_->streams[nVideoIndex];
            AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
            if(nullptr == codec){
                qDebug()<<"Couldn't find video codec."<<endl;
                break;
            }
            //创建编码器上下文
            if(nullptr == (mCodecCtx_ = avcodec_alloc_context3(codec))){
                qDebug()<<"create avcodec context failed."<<endl;
                break;
            }
            //初始化编码器上下文，用AVStream里的参数初始化AVCodecContext
            if(0 > avcodec_parameters_to_context(mCodecCtx_, stream->codecpar)){
                qDebug()<<"Couldn't alloc AVCodecContext."<<endl;
                break;
            }

            if(0 != avcodec_open2(mCodecCtx_, codec, nullptr)){
                qDebug()<<"open avcodec failed."<<endl;
                break;
            }
            isCaptureThreadExit = false;

            CaptureThreadData *data = new CaptureThreadData(this->shared_from_this());
            std::thread([](void* data){
                CaptureThreadData *data_ = (CaptureThreadData*)data;
                data_->thiz_->funcThreadCapture();
                delete data;
            }, data).detach();
            return true;
        }while(false);
        if(mCodecCtx_){
            avcodec_free_context(&mCodecCtx_);
        }

        if(mFormatCtx_){
            avformat_close_input(&mFormatCtx_);
            avformat_free_context(mFormatCtx_);
        }
        return false;
    }

    bool stop(){
        return true;
    }

    void funcThreadCapture(){
        AVPacket *packet_ = av_packet_alloc();
        AVFrame *frame_ = av_frame_alloc();
        int result = -1;
        while(!isCaptureThreadExit){
            if(av_read_frame(mFormatCtx_, packet_) < 0){
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            if(packet_->stream_index == nVideoIndex){
                result = avcodec_send_packet(mCodecCtx_, packet_);
                if(result < 0 && result != AVERROR_EOF){
                    av_packet_unref(packet_);
                    continue;
                }
                result = avcodec_receive_frame(mCodecCtx_, frame_);
                if(result < 0 && result != AVERROR_EOF){
                    av_packet_unref(packet_);
                    continue;
                }
            }
            av_packet_unref(packet_);
        }
    }

private:
    bool isinit_ = false;
    bool isCaptureThreadExit = true;
    InitParam init_param_;
    AVFormatContext *mFormatCtx_ = nullptr;
    AVCodecContext *mCodecCtx_ = nullptr;
    int nVideoIndex = -1;
};



FFmpegWrapper::FFmpegWrapper(){
    impl_ = std::make_shared<Impl>();
}

FFmpegWrapper::~FFmpegWrapper(){
    impl_ = nullptr;
}

bool FFmpegWrapper::Init(InitParam& param){
    return impl_->Init(param);
}

void FFmpegWrapper::unInit(){
    return impl_->unInit();
}

bool FFmpegWrapper::start(){
    return impl_->start();
}

bool FFmpegWrapper::stop(){
    return impl_->stop();
}
