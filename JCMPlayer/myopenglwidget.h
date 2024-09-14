#ifndef  MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QTimer>

static const char *vertexShaderSource =
    "attribute highp vec4 posAttr;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform highp mat4 matrix;\n"

    "attribute vec2 a_texCoord;\n"
    "uniform mat4 u_matrix;\n"
    "varying vec2 v_texCoord;\n"

    "void main() {\n"
    "   col = colAttr;\n"
    "   v_texCoord=a_texCoord;"

    "   gl_Position = matrix * posAttr;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying lowp vec4 col;\n"

    "varying vec2 v_texCoord;\n"
    "uniform sampler2D u_texture;\n"

    "void main() {\n"
    "   gl_FragColor = col;\n"
    "}\n";

class QOpenGLShaderProgram;
class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    explicit MyOpenGLWidget(QWidget *parent = 0);
    ~MyOpenGLWidget();

public:
    void StartAnimating();
protected:
    /*初始化OpenGL资源和状态*/
    void initializeGL();
    /*渲染OpenGL场景*/
    void paintGL();
    /*设置OpenGL的视口、投影等。每次部件大小改变时都会调用该函数*/
    void resizeGL(int width, int height);

private:
    GLint m_posAttr = 0;
    GLint m_colAttr = 0;
    GLint m_matrixUniform = 0;

    QOpenGLShaderProgram *m_program = nullptr;

    int m_frame = 0;

    QTimer *m_timer;
};
#endif
