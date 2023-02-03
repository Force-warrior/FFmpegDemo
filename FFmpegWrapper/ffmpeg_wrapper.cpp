#include "ffmpeg_wrapper.h"

FFmpegWrapper::FFmpegWrapper()
    :init_(false)
{

}

FFmpegWrapper::~FFmpegWrapper(){

}

bool FFmpegWrapper::Init(InitParam& param){
    if(init_) return true;
    memcpy(&init_param_, &param, sizeof (param));
    avdevice_register_all();
    init_ = true;
    return true;
}

void FFmpegWrapper::unInit(){
    init_ = false;
}

bool FFmpegWrapper::start(){
    AVInputFormat *iformat = nullptr;
    AVDictionary* options = nullptr;
    iformat = av_find_input_format("dshow");
    if (nullptr == iformat) {
         return false;
    }

    char resolution[32];
    sprintf(resolution, "%dx%d", init_param_.video_width, init_param_.video_height);
    av_dict_set(&options, "video_size", resolution, 0);

    return true;
}

bool FFmpegWrapper::stop(){
    return true;
}
