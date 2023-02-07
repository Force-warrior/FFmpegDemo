#include "ffmpeg_wrapper.h"

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

#include <thread>
#include <memory>
#include <mutex>

#include <QDebug>

static inline char* err2str(int eno){
    static char buf[1024];
    return av_make_error_string(buf, size_t(sizeof(buf)), eno);
}
struct MatchAVPIX{
    PIXEL_FORMAT iformat;
    AVPixelFormat avformat;
};
static MatchAVPIX AV_PIX_FMT[]={
            {FMT_RGBA, AV_PIX_FMT_RGBA},
            {FMT_BGRA, AV_PIX_FMT_BGRA},
            {FMT_I420,AV_PIX_FMT_YUV420P},
            {FMT_NV12,AV_PIX_FMT_NV12}};

class FFmpegWrapper::Impl: public enable_shared_from_this<Impl>{
public:
    struct CaptureThreadData{
        CaptureThreadData(std::shared_ptr<Impl> data): thiz_(data){
        }
        const std::shared_ptr<Impl> thiz_;
    };

    struct OutputContext{
        VideoCaptureOutPut* handler;
        struct SwsContext *convert;
    };

public:
    Impl()
        :isinit_(false),
        isCaptureThreadExit(false),
        mOutputAVFrame(nullptr),
        mCodecCtx_(nullptr),
        mFormatCtx_(nullptr){

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
        release();
    }

    void release(){
        if(mOutputAVFrame){
            av_frame_free(&mOutputAVFrame);
            mOutputAVFrame = nullptr;
        }

        if(mCodecCtx_){
            avcodec_free_context(&mCodecCtx_);
        }

        if(mFormatCtx_){
            avformat_close_input(&mFormatCtx_);
            avformat_free_context(mFormatCtx_);
        }
    }

    bool startCapture(){
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

            {
                CaptureThreadData *data = new CaptureThreadData(this->shared_from_this());
                std::thread([](void* data){
                    CaptureThreadData *data_ = (CaptureThreadData*)data;
                    data_->thiz_->funcThreadCapture();
                    delete data;
                }, data).detach();
            }

            {
                CaptureThreadData *data = new CaptureThreadData(this->shared_from_this());
                std::thread([](void* data){
                    CaptureThreadData *data_ = (CaptureThreadData*)data;
                    data_->thiz_->captureOutputFunc();
                    delete data;
                }, data).detach();
            }

            return true;
        }while(false);
        release();
        return false;
    }

    bool stopCapture(){
        return true;
    }

    void funcThreadCapture(){
        AVPacket *packet_ = av_packet_alloc();
        AVFrame *pframe_ = av_frame_alloc();
        AVFrame *poutframe_ = av_frame_alloc();
        int result = -1;

        //设置帧数据转换上下文
        struct SwsContext *img_convert_ctx = nullptr;
        img_convert_ctx = sws_getContext(mCodecCtx_->width, mCodecCtx_->height, AV_PIX_FMT_YUV420P,
                                         320, 240, AV_PIX_FMT_YUV420P,
                                         SWS_BILINEAR, NULL, NULL, NULL);

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
                result = avcodec_receive_frame(mCodecCtx_, pframe_);
                if(result < 0 && result != AVERROR_EOF){
                    av_packet_unref(packet_);
                    continue;
                }


                {
                    std::lock_guard<std::mutex> g_lock(mOutputAVFrameMutex);
                    if(mOutputAVFrame)
                        av_frame_free(&mOutputAVFrame);
                    mOutputAVFrame = av_frame_clone(pframe_);
                }

                if(!poutframe_){
                    av_packet_unref(packet_);
                    continue;
                }
            }
            av_packet_unref(packet_);
        }

        if(pframe_){
            av_frame_free(&pframe_);
            pframe_ = nullptr;
        }
        release();
    }

    void captureOutputFunc(){
        while(!isCaptureThreadExit){
            AVFrame* tempframe = nullptr;
            {
                std::lock_guard<std::mutex> g_lock(mOutputAVFrameMutex);
                if(!mOutputAVFrame)
                    continue;
                tempframe = av_frame_clone(mOutputAVFrame);
                av_frame_free(&mOutputAVFrame);
                mOutputAVFrame = nullptr;
            }
            if(!tempframe)
                continue;
            outputVideoFrame(tempframe);

            av_frame_free(&tempframe);
            tempframe = nullptr;
        }
    }
    void outputVideoFrame(AVFrame* frame){
        std::lock_guard<std::mutex> g_lock(mOutputContextMutex);
        for(auto iter = vec_output.begin(); iter != vec_output.end(); iter++){
            AVFrame *frameYUV = nullptr;
            if((frame->width == iter->handler->width) &&
               (frame->height == iter->handler->height) &&
               (AV_PIX_FMT[iter->handler->type].avformat == frame->format)){
                frameYUV = frame;
            }
            else{
                if(nullptr == iter->convert){
                    iter->convert = sws_getContext(frame->width, frame->height, mCodecCtx_->pix_fmt, iter->handler->width, iter->handler->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
                    if (!iter->convert) {
                        continue;
                    }
                }
                if (frameYUV == nullptr) {
                    frameYUV = av_frame_alloc();
                    if (!frameYUV)
                        continue;

                    frameYUV->format = AV_PIX_FMT[iter->handler->type].avformat;
                    frameYUV->width = iter->handler->width;
                    frameYUV->height = iter->handler->height;

                    /* allocate the buffers for the frame data */
                    int ret = av_frame_get_buffer(frameYUV, 32);
                    if (ret < 0) {
                        av_frame_free(&frameYUV);
                        frameYUV = nullptr;
                        continue;
                    }
                }

                // convert pix_fmt only
                int ret = sws_scale(iter->convert, (const unsigned char* const*)frame->data, frame->linesize, 0, mCodecCtx_->height, frameYUV->data, frameYUV->linesize);
            }
            VideoFrame outputvideoframe;
            outputvideoframe.data[0] = frameYUV->data[0];
            outputvideoframe.data[1] = frameYUV->data[1];
            outputvideoframe.data[2] = frameYUV->data[2];
            outputvideoframe.linesize[0] = frameYUV->linesize[0];
            outputvideoframe.linesize[1] = frameYUV->linesize[1];
            outputvideoframe.linesize[2] = frameYUV->linesize[2];

            outputvideoframe.width = frameYUV->width;
            outputvideoframe.height = frameYUV->height;
            outputvideoframe.pts = frameYUV->pts;
            //outputvideoframe.framerate = frameYUV->;
            iter->handler->eventhandler->onCaptureVideoFrame(outputvideoframe);
        }
    }
    void addOutPut(VideoCaptureOutPut* output){
        std::lock_guard<std::mutex> g_lock(mOutputContextMutex);
        OutputContext oc;
        oc.handler = output;
        oc.convert = nullptr;
        vec_output.push_back(oc);
    }

    void delOutPut(VideoCaptureOutPut* output){
        std::lock_guard<std::mutex> g_lock(mOutputContextMutex);
        for(auto iter = vec_output.begin(); iter != vec_output.end(); iter++){
            if(iter->handler == output){
                if(iter->convert)
                    sws_freeContext(iter->convert);
                vec_output.erase(iter++);
                return;
            }
        }
    }

private:
    bool isinit_ = false;
    bool isCaptureThreadExit = true;
    InitParam init_param_;
    AVFormatContext *mFormatCtx_ = nullptr;
    AVCodecContext *mCodecCtx_ = nullptr;
    AVFrame *mOutputAVFrame = nullptr;
    int nVideoIndex = -1;
    std::mutex mOutputAVFrameMutex;
    std::mutex mOutputContextMutex;
    std::vector<OutputContext> vec_output;
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

bool FFmpegWrapper::startCapture(){
    return impl_->startCapture();
}

bool FFmpegWrapper::stopCapture(){
    return impl_->stopCapture();
}

void FFmpegWrapper::addOutPut(VideoCaptureOutPut* output){
    return impl_->addOutPut(output);
}

void FFmpegWrapper::delOutPut(VideoCaptureOutPut* output){
    return impl_->delOutPut(output);
}
