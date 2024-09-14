#ifndef VIDEOOPENGL_H
#define VIDEOOPENGL_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions>


QT_BEGIN_NAMESPACE
class QOpenGLTexture;
class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
QT_END_NAMESPACE


namespace common {

namespace qt {

class VideoOpenGL : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    enum ImageFormat{
        IMAGE_FORMAT_Unknown,
        IMAGE_FORMAT_NV12,      // y... uvuv...
        IMAGE_FORMAT_NV21,      // y... vuvu...
        IMAGE_FORMAT_I420,      // y... u... v...
        IMAGE_FORMAT_RGB32,     // b g r a
        IMAGE_FORMAT_ARGB32,    // b g r a
        IMAGE_FORMAT_RGB888,    // r g b
        IMAGE_FORMAT_Grayscale8,// grayscale8
        IMAGE_FORMAT_BGR888,    // b g r(OpenCV Mat Format)
    };
    VideoOpenGL(QWidget* parent = nullptr);
    ~VideoOpenGL();
    void set_image_size(const int& width, const int& height, const ImageFormat& farmat);
    void update_image(char* data_ptr);


protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void paintEvent(QPaintEvent *e) override;

private:
    struct impl;
    struct impl* m;
};

}

}

#endif // VIDEOOPENGL_H




