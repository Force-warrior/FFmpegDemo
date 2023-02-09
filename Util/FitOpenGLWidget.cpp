#include "FitOpenGLWidget.h"
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <mutex>

class FitOpenGLWidget::FitOpenGLWidgetImpl{
public:
    FitOpenGLWidgetImpl(){

    }

    ~FitOpenGLWidgetImpl(){
        if (!!textureY_) {
            textureY_->destroy();
            delete  textureY_;
        }
        if (!!textureU_) {
            textureU_->destroy();
            delete  textureU_;
        }
        if (!!textureV_) {
            textureV_->destroy();
            delete  textureV_;
        }

    }

    void UpdateVideoFrameYUVI420(uint8_t** yuvData,  uint32_t yuvWidth, uint32_t yuvHeight, int* yuvLinesize){
        std::unique_lock<std::mutex> lock(yuvBufferCS_);

        videoW_ = yuvWidth;
        videoH_ = yuvHeight;
        // 被8整除
        int remainder = videoW_ % 8;
        if (remainder != 0) {
            videoW_ += 8 - (remainder);
        }
        if (videoH_ % 8 != 0) {
//            yuvHeight_ += 8 - (yuvHeight_ % 8);
        }
        unsigned bsize = videoW_ * videoH_ * 3 / 2;
        if (yuvBufferSize_ != bsize)
        {
            yuvBufferSize_ = bsize;
            delete[] reinterpret_cast<unsigned char*>(yuvPtr_);
            yuvPtr_ = nullptr;
        }
        if (yuvBufferSize_ == 0)
            return;

        if (yuvPtr_ == nullptr) {
            yuvPtr_ =  new unsigned char[yuvBufferSize_];
        }

        if (!yuvPtr_  || !yuvData || !yuvLinesize) {
            return ;
        }

        uint8_t *destPtr = (uint8_t *)yuvPtr_;
        uint8_t *yData = yuvData[0];
        uint8_t *uData = yuvData[1];
        uint8_t *vData = yuvData[2];

        memset(yuvPtr_, 0, yuvBufferSize_);
        // Y
        for (int i=0; i<yuvHeight; ++i) {
            memcpy(destPtr, yData, yuvWidth);
            destPtr += videoW_;
            yData += yuvLinesize[0];
        }
        // U
        for (int i=0; i<yuvHeight/2; ++i) {
            memcpy(destPtr, uData, yuvWidth/2);
            destPtr += videoW_/2;
            uData += yuvLinesize[1];
        }
        // V
        for (int i=0; i<yuvHeight/2; ++i) {
            memcpy(destPtr, vData, yuvWidth/2);
            destPtr += videoW_/2;
            vData += yuvLinesize[2];
        }
        return ;
    }

public:
    QOpenGLTexture *textureY_ = nullptr,*textureU_ = nullptr,*textureV_ = nullptr;
    QOpenGLShaderProgram*   shaderProgram_ = nullptr;
    unsigned int    yuvBufferSize_ = 0;

    GLuint textureUniformY_, textureUniformU_, textureUniformV_; //opengl中y、u、v分量位置
    GLsizei videoW_, videoH_;
    GLuint idY_, idU_, idV_; //自己创建的纹理对象ID，创建错误返回0
    uint8_t *yuvPtr_ = nullptr;
    QOpenGLBuffer vbo_;

    std::mutex yuvBufferCS_;

};

#define VERTEXIN 0
#define TEXTUREIN 1

FitOpenGLWidget::FitOpenGLWidget(QWidget *paraent, Qt::WindowFlags flags){
    impl_ = std::make_shared<FitOpenGLWidgetImpl>();
}

FitOpenGLWidget::~FitOpenGLWidget(){

}

void FitOpenGLWidget::initializeGL(){
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);         //启用OpenGL深度测试

    static const GLfloat vertices[]{
        //顶点坐标
        -1.0f,-1.0f,
        -1.0f,+1.0f,
        +1.0f,+1.0f,
        +1.0f,-1.0f,
        //纹理坐标
        0.0f,1.0f,
        0.0f,0.0f,
        1.0f,0.0f,
        1.0f,1.0f,
    };

    impl_->vbo_.create();
    impl_->vbo_.bind();
    impl_->vbo_.allocate(vertices,sizeof(vertices));

    //初始化顶点着色器对象
    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex,this);
    //顶点着色器源码
    const char *vsrc =
       "attribute vec4 vertexIn; \
        attribute vec2 textureIn; \
        varying vec2 textureOut;  \
        void main(void)           \
        {                         \
            gl_Position = vertexIn; \
            textureOut = textureIn; \
        }";
    //编译顶点着色器程序
    if(!vshader->compileSourceCode(vsrc)){
        qDebug()<<"vshader compileSourceCode failed!"<<endl;
    }

    //初始化片段着色器 功能gpu中yuv转换成rgb
    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment,this);
    //片段着色器源码
    const char *fsrc =
        "varying vec2 textureOut; \
        uniform sampler2D tex_y; \
        uniform sampler2D tex_u; \
        uniform sampler2D tex_v; \
        void main(void) \
        { \
            vec3 yuv; \
            vec3 rgb; \
            yuv.x = texture2D(tex_y, textureOut).r; \
            yuv.y = texture2D(tex_u, textureOut).r - 0.5; \
            yuv.z = texture2D(tex_v, textureOut).r - 0.5; \
            rgb = mat3( 1,       1,         1, \
                        0,       -0.39465,  2.03211, \
                        1.13983, -0.58060,  0) * yuv; \
            gl_FragColor = vec4(rgb, 1); \
        }";
    //将glsl源码送入编译器编译着色器程序
    if(!fshader->compileSourceCode(fsrc)){
        qDebug()<<"fshader compileSourceCode failed!"<<endl;
    }

    impl_->shaderProgram_ = new QOpenGLShaderProgram(this);
    //将顶点着色器添加到程序容器
    impl_->shaderProgram_->addShader(vshader);
    //将片段着色器添加到程序容器
    impl_->shaderProgram_->addShader(fshader);
    //绑定属性vertexIn到指定位置ATTRIB_VERTEX,该属性在顶点着色源码其中有声明
    impl_->shaderProgram_->bindAttributeLocation("vertexIn",VERTEXIN);
    //绑定属性textureIn到指定位置ATTRIB_TEXTURE,该属性在顶点着色源码其中有声明
    impl_->shaderProgram_->bindAttributeLocation("textureIn",TEXTUREIN);
    //链接所有所有添入到的着色器程序
    impl_->shaderProgram_->link();
    //激活所有链接
    impl_->shaderProgram_->bind();

    impl_->shaderProgram_->enableAttributeArray(VERTEXIN);
    impl_->shaderProgram_->enableAttributeArray(TEXTUREIN);
    impl_->shaderProgram_->setAttributeBuffer(VERTEXIN,GL_FLOAT,0,2,2*sizeof(GLfloat));
    impl_->shaderProgram_->setAttributeBuffer(TEXTUREIN,GL_FLOAT,8*sizeof(GLfloat),2,2*sizeof(GLfloat));

    //读取着色器中的数据变量tex_y, tex_u, tex_v的位置,这些变量的声明可以在片段着色器源码中可以看到
    impl_->textureUniformY_ = impl_->shaderProgram_->uniformLocation("tex_y");
    impl_->textureUniformU_ = impl_->shaderProgram_->uniformLocation("tex_u");
    impl_->textureUniformV_ = impl_->shaderProgram_->uniformLocation("tex_v");

    //分别创建y,u,v纹理对象
    impl_->textureY_ = new QOpenGLTexture(QOpenGLTexture::Target2D);
    impl_->textureU_ = new QOpenGLTexture(QOpenGLTexture::Target2D);
    impl_->textureV_ = new QOpenGLTexture(QOpenGLTexture::Target2D);
    impl_->textureY_->create();
    impl_->textureU_->create();
    impl_->textureV_->create();

    //返回各纹理对象的纹理索引值
    impl_->idY_ = impl_->textureY_->textureId();
    impl_->idU_ = impl_->textureU_->textureId();
    impl_->idV_ = impl_->textureV_->textureId();

    //设置背景色
    glClearColor(0.0,0.0,0.0,0.0);
}

void FitOpenGLWidget::resizeGL(int width, int height){
    glViewport(0, 0, width, height);
    update();
}

void FitOpenGLWidget::paintGL(){
    std::unique_lock<std::mutex> lock(impl_->yuvBufferCS_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (nullptr == impl_->yuvPtr_)
        return ;

    //------- 加载y数据纹理 -------
    glActiveTexture(GL_TEXTURE0);  //**纹理单元GL_TEXTURE0,系统里面的
    glBindTexture(GL_TEXTURE_2D, impl_->idY_); //绑定y分量纹理对象id到**的纹理单元
    //使用内存中的数据创建真正的y分量纹理数据
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED, impl_->videoW_, impl_->videoH_, 0, GL_RED,GL_UNSIGNED_BYTE, impl_->yuvPtr_);
    //https://blog.csdn.net/xipiaoyouzi/article/details/53584798 纹理参数解析
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //------- 加载u数据纹理 -------
    glActiveTexture(GL_TEXTURE1); //**纹理单元GL_TEXTURE1
    glBindTexture(GL_TEXTURE1, impl_->idU_);
    //使用内存中的数据创建真正的u分量纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,impl_->videoW_ >> 1, impl_->videoH_ >> 1,0,GL_RED,GL_UNSIGNED_BYTE, impl_->yuvPtr_ + impl_->videoW_ * impl_->videoH_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //------- 加载v数据纹理 -------
    glActiveTexture(GL_TEXTURE2); //**纹理单元GL_TEXTURE2
    glBindTexture(GL_TEXTURE_2D, impl_->idV_);
    //使用内存中的数据创建真正的v分量纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, impl_->videoW_ >> 1, impl_->videoH_ >> 1, 0, GL_RED, GL_UNSIGNED_BYTE, impl_->yuvPtr_ + impl_->videoW_ * impl_->videoH_ * 5 / 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //指定y纹理要使用新值
    glUniform1i(impl_->textureUniformY_, 0);
    //指定u纹理要使用新值
    glUniform1i(impl_->textureUniformU_, 1);
    //指定v纹理要使用新值
    glUniform1i(impl_->textureUniformV_, 2);

    //使用顶点数组方式绘制图形
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void FitOpenGLWidget::UpdateVideoFrameYUVI420(uint8_t** yuvData,  uint32_t yuvWidth, uint32_t yuvHeight, int* yuvLinesize){
    impl_->UpdateVideoFrameYUVI420(yuvData, yuvWidth, yuvHeight, yuvLinesize);
    update();
}
