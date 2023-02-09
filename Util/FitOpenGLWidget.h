#ifndef FITOPENGLWIDGET_H
#define FITOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPaintEvent>

class FitOpenGLWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
public:
    FitOpenGLWidget(QWidget *paraent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~FitOpenGLWidget();
public:
    void UpdateVideoFrameYUVI420(uint8_t** yuvData,  uint32_t yuvWidth, uint32_t yuvHeight, int* yuvLinesize);

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int width, int height) override;
    virtual void paintGL() override;

private:
    class FitOpenGLWidgetImpl;
    std::shared_ptr<FitOpenGLWidgetImpl> impl_;
};

#endif // FITOPENGLWIDGET_H
