#ifndef FFMPEGWRAPPER_H
#define FFMPEGWRAPPER_H
#include <string.h>
#include <stdio.h>

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
    bool init_;
    InitParam init_param_;
};

#endif // FFMPEGWRAPPER_H
