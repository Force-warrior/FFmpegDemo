QT       += core gui
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    DEFINES += DEBUG

    UI_DIR = $$PWD/tmp/debug/ui
    MOC_DIR = $$PWD/tmp/debug/moc
    OBJECTS_DIR = $$PWD/tmp/debug/objects
    RCC_DIR = $$PWD/tmp/debug/resources

    DESTDIR = $$shell_path($$PWD/bin/debug)
} else {
    DEFINES -= _DEBUG
    DEFINES -= DEBUG
    DEFINES += NODEBUG
    DEFINES += NDEBUG

    UI_DIR = $$PWD/tmp/release/ui
    MOC_DIR = $$PWD/tmp/release/moc
    OBJECTS_DIR = $$PWD/tmp/release/objects
    RCC_DIR = $$PWD/tmp/release/resources

    QMAKE_CFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_OBJECTIVE_CFLAGS_RELEASE = $$QMAKE_OBJECTIVE_CFLAGS_RELEASE_WITH_DEBUGINFO
    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO

    DESTDIR = $$shell_path($$PWD/bin/release)
}

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

include($$PWD/FFmpegWrapper/ffmpeg_wrapper.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
