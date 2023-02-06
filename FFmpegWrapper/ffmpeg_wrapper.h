#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H
#include <string.h>
#include <stdio.h>
#include <memory>

#include "ffmpeg_def.h"
using namespace std;

class FFmpegWrapper
{
public:
    FFmpegWrapper();
    ~FFmpegWrapper();
public:
    bool Init(InitParam& param);
    void unInit();
    bool start();
    bool stop();

private:
    class Impl;
    std::shared_ptr<Impl> impl_;
};

#endif // FFMPEGWRAPPER_H
