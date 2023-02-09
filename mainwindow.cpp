#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCameraInfo>
#include <thread>
#include "ffmpeg_wrapper.h"
#include "IFFmpegDelegate.h"


#include <QDebug>
#include <QFile>
#include <QTimer>

class VideoCaptureHandler: public IVideoCaptureHandler{
public:
    VideoCaptureHandler(FitOpenGLWidget* widget):show_widget_(widget){}
    virtual ~VideoCaptureHandler(){}
    virtual void onCaptureVideoFrame(VideoFrame& frame) override{
        show_widget_->UpdateVideoFrameYUVI420(frame.data, frame.width, frame.height, frame.linesize);


//        auto ba = new QByteArray(320*240*1.5,0);
//        auto file = new QFile("d:\\picture.yuv");
//        file->open(QIODevice::ReadOnly);
//        while(1){
//            const int w=320,h=240;
//            if(file->read(ba->data(),ba->count()) <=0){
//                file->seek(0);
//                if(file->read(ba->data(),ba->count()) <=0){
//                    qDebug("error again");
//                    return;
//                }
//            }
//            show_widget_->UpdateVideoFrameYUVI420((uint8_t*)ba->data(), w, h, NULL);
//            std::this_thread::sleep_for(std::chrono::milliseconds(20));
//        }


//        FILE * file = NULL;
//        file = fopen("d:\\picture.yuv", "ab+");
//        unsigned char *py = (unsigned char*)frame.data[0];
//        unsigned char *pu = (unsigned char*)frame.data[1];
//        unsigned char *pv = (unsigned char*)frame.data[2];
//        for (int i = 0; i < frame.height; i++)
//        {
//            fwrite(py, frame.width, 1, file);
//            py += frame.linesize[0];
//        }
//        for (int i = 0; i < frame.height / 2; i++)
//        {
//            fwrite(pu, frame.width / 2, 1, file);
//            pu += frame.linesize[1];
//        }
//        for (int i = 0; i < frame.height / 2; i++)
//        {
//            fwrite(pv, frame.width / 2, 1, file);
//            pv += frame.linesize[2];
//        }
//        fclose(file);
    }

private:
    FitOpenGLWidget* show_widget_;
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
    VideoCaptureHandler *handler = new VideoCaptureHandler(ui->openGLWidget);
    VideoCaptureOutPut *output = new VideoCaptureOutPut;
    output->eventhandler = handler;
    output->width = 320;
    output->height = 240;
    output->type = FMT_I420;


    wrapper->addOutPut(output);


    InitParam initparam;
    initparam.in_video_width = 320;
    initparam.in_video_height = 240;
    initparam.in_pixel_fmt = FMT_I420;
#if 1
    initparam.in_capture_type = CAPTURE_DEVICE;
    strcpy(initparam.in_dev_or_url_, cur_dev_video.toLocal8Bit());
#else
    initparam.in_capture_type = CAPTURE_FILE;
    //QString file_path =  QCoreApplication::applicationDirPath() + "/Frozen.mp4";
    QString file_path = "http://pull-flv-l6.douyincdn.com/third/stream-112467449361989671_ld.flv?k=54bff2aa199b8373&t=1676514875";
    strcpy(initparam.in_dev_or_url_, file_path.toLocal8Bit());
#endif
    wrapper->Init(initparam);
    wrapper->startCapture();
}

