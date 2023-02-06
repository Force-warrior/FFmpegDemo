#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCameraInfo>

#include "ffmpeg_wrapper.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , wrapper(nullptr)
{
    ui->setupUi(this);
    getCameraList();
    if(!wrapper)
        wrapper= new FFmpegWrapper();
}

MainWindow::~MainWindow()
{
    if(wrapper){
        delete wrapper;
        wrapper = nullptr;
    }
    delete ui;
}

void MainWindow::getCameraList(){
    ui->cbx_dev_video->clear();
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for(auto camera : cameras)
    {
#if defined(Q_OS_WIN)
        // ffmpeg在Window下要使用video=description()
        ui->cbx_dev_video->addItem("video=" + camera.description());
#elif defined(Q_OS_LINUX)
                                                                  // ffmpeg在linux下要使用deviceName()
#elif defined(Q_OS_MAC)
#endif
    }
}

void MainWindow::on_cbx_dev_video_currentIndexChanged(const QString &arg){
    cur_dev_video = arg;
}

void MainWindow::on_btn_start_clicked(){
    InitParam initparam;

    initparam.in_video_width = 320;
    initparam.in_video_height = 240;
#if 1
    initparam.in_capture_type = CAPTURE_DEVICE;
    strcpy(initparam.in_path_file_name_, cur_dev_video.toLocal8Bit());
#else
    initparam.in_capture_type = CAPTURE_FILE;
    QString file_path =  QCoreApplication::applicationDirPath() + "/Frozen.mp4";
    strcpy(initparam.in_path_file_name_, file_path.toLocal8Bit());
#endif
    wrapper->Init(initparam);
    wrapper->start();
}

