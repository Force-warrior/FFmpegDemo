#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCameraInfo>

#include "ffmpeg_wrapper.h"
#include "IFFmpegDelegate.h"

#include <QDebug>

class VideoCaptureHandler: public IVideoCaptureHandler{
public:
    VideoCaptureHandler(){}
    virtual ~VideoCaptureHandler(){}
    virtual void onCaptureVideoFrame(VideoFrame& frame) override{
        FILE * file = NULL;
        file = fopen("d:\\picture.yuv", "ab+");
        unsigned char *py = (unsigned char*)frame.data[0];
        unsigned char *pu = (unsigned char*)frame.data[1];
        unsigned char *pv = (unsigned char*)frame.data[2];
        for (int i = 0; i < frame.height; i++)
        {
            fwrite(py, frame.width, 1, file);
            py += frame.linesize[0];
        }
        for (int i = 0; i < frame.height / 2; i++)
        {
            fwrite(pu, frame.width / 2, 1, file);
            pu += frame.linesize[1];
        }
        for (int i = 0; i < frame.height / 2; i++)
        {
            fwrite(pv, frame.width / 2, 1, file);
            pv += frame.linesize[2];
        }
        fclose(file);
    }
};



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
    VideoCaptureHandler *handler = new VideoCaptureHandler;
    VideoCaptureOutPut *output = new VideoCaptureOutPut;
    output->eventhandler = handler;
    output->width = 320;
    output->height = 240;
    output->type = FMT_I420;


    wrapper->addOutPut(output);


    InitParam initparam;
    initparam.in_video_width = 320;
    initparam.in_video_height = 240;
#if 0
    initparam.in_capture_type = CAPTURE_DEVICE;
    strcpy(initparam.in_path_file_name_, cur_dev_video.toLocal8Bit());
#else
    initparam.in_capture_type = CAPTURE_FILE;
    //QString file_path =  QCoreApplication::applicationDirPath() + "/Frozen.mp4";
    QString file_path = "http://pull-f3.douyincdn.com/third/stream-112455760474276523_ld.flv?auth_key=1676364706-0-0-30bb0f6b1b74ee0736997f14a8c17b8e";
    strcpy(initparam.in_path_file_name_, file_path.toLocal8Bit());
#endif
    wrapper->Init(initparam);
    wrapper->startCapture();
}

