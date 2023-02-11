#ifndef FFMPEG_DEF_H
#define FFMPEG_DEF_H

#include <stdint.h>

enum Capture_Tye{
    CAPTURE_DEVICE,
    CAPTURE_SCREEN,
    CAPTURE_FILE,
};

enum PIXEL_FORMAT {
    FMT_RGBA = 0,
    FMT_BGRA = 1,
    FMT_I420 = 2,
    FMT_NV12 = 3,
    FMT_MJPGFILE = 4 // take snapshot video picture only
};

struct InitParam{
    Capture_Tye in_capture_type;
    PIXEL_FORMAT in_pixel_fmt;  //only support I420 now
    char in_dev_or_url_[256]={0};  // 全路径文件名（utf-8）
    int32_t in_video_width = 0;
    int32_t in_video_height = 0;
    int32_t framerate = 0;

};

enum PLANE_TYPE {
    Y_PLANE = 0,
    U_PLANE = 1,
    V_PLANE = 2,
    NUM_OF_PLANES = 3
};

struct VideoFrame{
    PIXEL_FORMAT fmt;  //only support I420 now
    uint8_t *data[NUM_OF_PLANES];   //YUV: 0-y, 1-u, 2-v,  RGB: 0-data
    int linesize[NUM_OF_PLANES];
    int32_t width = 0;
    int32_t height = 0;
    int64_t pts = 0;
    int32_t framerate = 0;
};

enum CODEC_TYPE {
    CODEC_TYPE_H264, // auto select, using hardware encoder first, else useing software encoder
    CODEC_TYPE_H264_SOFT, // use software encoder
    CODEC_TYPE_H265,
    CODEC_TYPE_H265_SOFT,
    CODEC_TYPE_H264_SOFT_WZ // use wzh264 software encoder
};

struct EncConfig {
    CODEC_TYPE codecType;
};

struct DecConfig {
    CODEC_TYPE codecType;
};

#endif // FFMPEG_DEF_H
