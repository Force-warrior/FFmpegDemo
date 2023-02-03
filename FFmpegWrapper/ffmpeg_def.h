#ifndef FFMPEG_DEF_H
#define FFMPEG_DEF_H

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

enum Capture_Tye{
    CAPTURE_DEVICE,
    CAPTURE_SCREEN,
};

struct InitParam{
    char path_file_name_[256];  // 全路径文件名（utf-8）
    Capture_Tye type;
    int video_width;
    int video_height;
};

#endif // FFMPEG_DEF_H
