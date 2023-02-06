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
    CAPTURE_FILE,
};

struct InitParam{
    Capture_Tye in_capture_type;
    char in_path_file_name_[256]={0};  // 全路径文件名（utf-8）
    int in_video_width;
    int in_video_height;

};

#endif // FFMPEG_DEF_H
