#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class FFmpegWrapper;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_cbx_dev_video_currentIndexChanged(const QString &arg);

    void on_btn_start_clicked();

private:
    void getCameraList();
private:
    Ui::MainWindow *ui;
    QString cur_dev_video;

    FFmpegWrapper *wrapper;
};
#endif // MAINWINDOW_H
