
HEADERS += \
    $$PWD/IFFmpegDelegate.h \
    $$PWD/ffmpeg_wrapper.h \
    $$PWD/ffmpeg_def.h

SOURCES += \
    $$PWD/ffmpeg_wrapper.cpp

INCLUDEPATH += \
    $$PWD \


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
