#include "VideoDecBase.h"
#include <string>

namespace VCodec {

VideoDecBase::VideoDecBase(){

}

VideoDecBase::~VideoDecBase(){

}

bool VideoDecBase::Initialize(IVideoDecHandler* handler, DecConfig& config) {
    memset(&mcontext_, 0, sizeof(mcontext_));
    mcontext_.handler_ = handler;
    mcontext_.config_ = config;
    return open_codec();
}

int VideoDecBase::reconfig(DecConfig& config){
    mcontext_.config_ = config;
    return 0;
}

int VideoDecBase::decode(uint8_t *data, int32_t len, int64_t timestamp){
    if(!data || len <= 0)
        return -1;
    mcontext_.inputdata_ = data;
    mcontext_.ndatasize_ = len;
    mcontext_.timestamp_ = timestamp;
    return start();
}

}
