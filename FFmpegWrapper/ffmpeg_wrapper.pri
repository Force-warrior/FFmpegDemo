
HEADERS += \
    $$PWD/IFFmpegDelegate.h \
    $$PWD/VideoCodec/decoder/DecWithFFmpegH264.h \
    $$PWD/VideoCodec/decoder/DecWithFFmpegSoft.h \
    $$PWD/VideoCodec/encoder/EncoderWithFFmpeg.h \
    $$PWD/VideoEncImpl.h \
    $$PWD/ffmpeg_wrapper.h \
    $$PWD/ffmpeg_def.h \
    $$PWD/VideoCodec/IVideoCodec.h \
    $$PWD/VideoCodec/decoder/VideoDecBase.h \
    $$PWD/VideoCodec/encoder/VideoEncBase.h \

SOURCES += \
    $$PWD/VideoCodec/decoder/DecWithFFmpegH264.cpp \
    $$PWD/VideoCodec/decoder/DecWithFFmpegSoft.cpp \
    $$PWD/VideoCodec/encoder/EncoderWithFFmpeg.cpp \
    $$PWD/ffmpeg_wrapper.cpp \
    $$PWD/VideoCodec/encoder/VideoEncBase.cpp \
    $$PWD/VideoCodec/decoder/VideoDecBase.cpp \

INCLUDEPATH += \
    $$PWD \
    $$PWD/VideoCodec \


win32:{
    INCLUDEPATH += \
            $$PWD/../third_party/ffmpeg/win32/include

    LIBS += \
        $$PWD/../third_party/ffmpeg/win32/lib/avcodec.lib    \
        $$PWD/../third_party/ffmpeg/win32/lib/avdevice.lib   \
        $$PWD/../third_party/ffmpeg/win32/lib/avformat.lib   \
        $$PWD/../third_party/ffmpeg/win32/lib/avformat.lib   \
        $$PWD/../third_party/ffmpeg/win32/lib/avutil.lib     \
        $$PWD/../third_party/ffmpeg/win32/lib/swresample.lib \
        $$PWD/../third_party/ffmpeg/win32/lib/swscale.lib    \
}
