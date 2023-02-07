#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H
#include <string.h>
#include <stdio.h>
#include <memory>

#include "ffmpeg_def.h"
#include "IFFmpegDelegate.h"

using namespace std;

class FFmpegWrapper
{
public:
    FFmpegWrapper();
    ~FFmpegWrapper();
public:
    bool Init(InitParam& param);
    void unInit();
    bool startCapture();
    bool stopCapture();
    void addOutPut(VideoCaptureOutPut* output);
    void delOutPut(VideoCaptureOutPut* output);

private:
    class Impl;
    std::shared_ptr<Impl> impl_;
};

#endif // FFMPEGWRAPPER_H
