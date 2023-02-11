#include "VideoEncBase.h"

namespace VCodec {

VideoEncBase::VideoEncBase(){

}

VideoEncBase::~VideoEncBase(){

}

bool VideoEncBase::Initialize(IVideoEncHandler* handler, EncConfig& config) {
    mhandler_ = handler;
    mconfig_ = config;
    return true;
}

int VideoEncBase::reconfig(EncConfig& config) {
    return 0;
}

int VideoEncBase::encode(VideoFrame *inframe, bool forceIFrmae) {
    return 0;
}

}
