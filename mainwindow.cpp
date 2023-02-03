#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCameraInfo>

#include "ffmpeg_wrapper.h"

#include <QDebug>

void getCameraList(){
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for(auto camera : cameras)
    {
#if defined(Q_OS_WIN)

        qDebug()<<"video=" + camera.description()<<endl;;    // ffmpeg在Window下要使用video=description()
#elif defined(Q_OS_LINUX)
                                                                  // ffmpeg在linux下要使用deviceName()
#elif defined(Q_OS_MAC)
#endif
    }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    getCameraList();
    FFmpegWrapper *wrapper= new FFmpegWrapper;
    InitParam initparam;
    initparam.type = CAPTURE_DEVICE;
    initparam.video_width = 320;
    initparam.video_height = 240;
    wrapper->Init(initparam);
    wrapper->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}
