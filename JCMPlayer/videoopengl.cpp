#include "videoopengl.h"
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <iostream>
#include <QPainter>
using namespace common::qt;

QByteArray version_shader_code(const char* src);
struct VideoOpenGL::impl
{
    impl() = default;
    ~impl(){
        if(this->m_video_ptr != nullptr){
            delete this->m_video_ptr;
            this->m_video_ptr = nullptr;
        }
        f->glDeleteTextures(1, &texture_id_y);
        f->glDeleteTextures(1, &texture_id_u);
        f->glDeleteTextures(1, &texture_id_v);
        m_vbo->release();
        m_program->release();
        delete m_vbo;
        delete m_program;
    }
    QOpenGLShaderProgram *m_program ;
    QOpenGLBuffer *m_vbo = nullptr;
    QOpenGLVertexArrayObject *m_vao = nullptr;
    GLuint texture_id_y, texture_id_u, texture_id_v;
    int m_video_w = 0, m_video_h = 0;
    int m_image_size = 0;
    char* m_video_ptr = nullptr;
    ImageFormat format = IMAGE_FORMAT_Unknown;
    QOpenGLFunctions *f;
};

VideoOpenGL::VideoOpenGL(QWidget* parent)
    : QOpenGLWidget(parent)
{
    m = new struct impl;
}


VideoOpenGL::~VideoOpenGL()
{
    makeCurrent();
    delete m;
}

QByteArray version_shader_code(const char *src)
{
    QByteArray versionedSrc;

    if (QOpenGLContext::currentContext()->isOpenGLES())
        versionedSrc.append(QByteArrayLiteral("#version 300 es\n"));
    else
        versionedSrc.append(QByteArrayLiteral("#version 330\n"));

    versionedSrc.append(src);
    return versionedSrc;
}

void VideoOpenGL::initializeGL()
{
    //显示视频流，一定不要开启深度测试！！！
    this->m->f = QOpenGLContext::currentContext()->functions();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    const char* vertex_shader_source = "layout(location = 0) in vec4 in_position;\n"
                                       "layout(location = 1) in vec2 in_texture;\n"
                                       "out vec2 out_texture;\n"
                                       "void main(void){\n"
                                       "    gl_Position = in_position;\n"
                                       "    out_texture = in_texture;\n"
                                       "}\n";

    // OpenGLES 内置矩阵mat3是一列一列的构建
    const char* fragment_shader_source = "in highp vec2 out_texture;\n"
                                         "uniform sampler2D texture_y;\n"
                                         "uniform sampler2D texture_u;\n"
                                         "uniform sampler2D texture_v;\n"
                                         "uniform int video_format;\n"
                                         "out highp vec4 fragColor;\n"
                                         "void main(){\n"
                                         "  highp vec3 yuv, rgb;\n"
                                         "  highp float alpha = 1.0;\n"
                                         "  if(video_format == 1){\n"
                                         "      yuv.x = texture(texture_y, out_texture).r;\n"
                                         "      yuv.y = texture(texture_u, out_texture).r-0.5;\n"
                                         "      yuv.z = texture(texture_u, out_texture).a-0.5;\n"
                                         "      rgb = mat3(1,       1,          1,  \
                                                           0,       -0.34414,   1.77216,\
                                                           1.40168, -0.71417,   0) * yuv;\n"
                                         "  }else if(video_format == 2){\n"
                                         "      yuv.x = texture(texture_y, out_texture).r;\n"
                                         "      yuv.y = texture(texture_u, out_texture).a-0.5;\n"
                                         "      yuv.z = texture(texture_u, out_texture).r-0.5;\n"
                                         "      rgb = mat3(1,       1,          1,  \
                                                             0,       -0.34414,   1.77216,\
                                                             1.40168, -0.71417,   0) * yuv;\n"
                                         "  }else if(video_format == 3){\n"
                                         "      yuv.x = texture(texture_y, out_texture).r;\n"
                                         "      yuv.y = texture(texture_u, out_texture).r-0.5;\n"
                                         "      yuv.z = texture(texture_v, out_texture).r-0.5;\n"
                                         "      rgb = mat3(1,       1,          1,  \
                                                           0,       -0.34414,   1.77216,\
                                                           1.40168, -0.71417,   0) * yuv;\n"
                                         "  }else if(video_format == 4){\n"
                                         "      highp vec4 tmp = texture(texture_y, out_texture);\n"
                                         "      rgb = vec3(tmp.b, tmp.g, tmp.r);\n"
                                         "  }else if(video_format == 5){\n"
                                         "      highp vec4 tmp = texture(texture_y, out_texture);\n"
                                         "      rgb = vec3(tmp.b, tmp.g, tmp.r);\n"
                                         "      alpha = tmp.a;\n"
                                         "  }else if(video_format == 6){\n"
                                         "      highp vec4 tmp = texture(texture_y, out_texture);\n"
                                         "      rgb = vec3(tmp.r, tmp.g, tmp.b);\n"
                                         "  }else if(video_format == 7){\n"
                                         "      highp vec4 tmp = texture(texture_y, out_texture);\n"
                                         "      rgb = tmp.rgb;\n"
                                         "  }else if(video_format == 8){\n"
                                         "      highp vec4 tmp = texture(texture_y, out_texture);\n"
                                         "      rgb = vec3(tmp.b, tmp.g, tmp.r);\n"
                                         "  }\n"
                                         "  fragColor = vec4(rgb, alpha);\n"
                                         "}\n";

    this->m->m_program = new QOpenGLShaderProgram;
    this->m->m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, version_shader_code(vertex_shader_source));
    this->m->m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, version_shader_code(fragment_shader_source));
    this->m->m_program->link();
    this->m->m_program->bind();
    this->m->m_program->setUniformValue("texture_y", 0);
    this->m->m_program->setUniformValue("texture_u", 1);
    this->m->m_program->setUniformValue("texture_v", 2);

#if 0
    static const GLfloat vertex_vertices[] = {
        -1.0f,  -1.0f,
         1.0f,  -1.0f,
        -1.0f,   1.0f,
         1.0f,   1.0f,
    };

    static const GLfloat texture_vertices[] = {
        0.0f,   1.0f,
        1.0f,   1.0f,
        0.0f,   0.0f,
        1.0f,   0.0f,
    };

    f->glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, vertex_vertices);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, texture_vertices);
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
#else
    static const GLfloat vertices[] = {
        -1.0f,  -1.0f,  0.0f,   1.0f,
         1.0f,  -1.0f,  1.0f,   1.0f,
        -1.0f,   1.0f,  0.0f,   0.0f,
         1.0f,   1.0f,  1.0f,   0.0f,
    };

    this->m->m_vao = new QOpenGLVertexArrayObject(this);
    this->m->m_vao->create();
    this->m->m_vao->bind();

    this->m->m_vbo = new QOpenGLBuffer;
    this->m->m_vbo->create();
    this->m->m_vbo->bind();
    this->m->m_vbo->allocate(vertices, sizeof(vertices));
    f->glEnableVertexAttribArray(0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), 0);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
                             reinterpret_cast<void *>(2*sizeof(float)));
#endif

#if 1
    f->glGenTextures(1, &this->m->texture_id_y);
    f->glGenTextures(1, &this->m->texture_id_u);
    f->glGenTextures(1, &this->m->texture_id_v);

    f->glActiveTexture(GL_TEXTURE0);
    f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_y);
    f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    f->glActiveTexture(GL_TEXTURE1);
    f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_u);
    f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    f->glActiveTexture(GL_TEXTURE2);
    f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_v);
    f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
    f->glBindTexture(GL_TEXTURE_2D, 0);
    this->m->m_vao->release();
    this->m->m_vbo->release();
    this->m->m_program->release();

    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_SRC_ALPHA_SATURATE);//GL_DST_ALPHA
}

void VideoOpenGL::resizeGL(int w, int h)
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(0,0,w,h==0?1:h);
}

void VideoOpenGL::paintGL()
{
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glClear(GL_COLOR_BUFFER_BIT);
    this->m->m_program->bind();
    this->m->m_vao->bind();
#if 1
    this->m->m_program->setUniformValue("video_format", this->m->format);
    switch (this->m->format)
    {
    case IMAGE_FORMAT_Unknown:
        std::cout << "[W] Unknow video farmat." << std::endl;
        return ;
    case IMAGE_FORMAT_NV12:
    case IMAGE_FORMAT_NV21:
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_y);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, this->m->m_video_w, this->m->m_video_h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, this->m->m_video_ptr);

        f->glActiveTexture(GL_TEXTURE1);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_u);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, this->m->m_video_w/2, this->m->m_video_h/2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, this->m->m_video_ptr+this->m->m_video_w*this->m->m_video_h);
        break;
    case IMAGE_FORMAT_I420:
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_y);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, this->m->m_video_w, this->m->m_video_h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, this->m->m_video_ptr);

        f->glActiveTexture(GL_TEXTURE1);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_u);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, this->m->m_video_w/2, this->m->m_video_h/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, this->m->m_video_ptr+this->m->m_video_w*this->m->m_video_h);

        f->glActiveTexture(GL_TEXTURE2);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_v);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, this->m->m_video_w/2, this->m->m_video_h/2, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, this->m->m_video_ptr+this->m->m_video_w*this->m->m_video_h*5/4);
        break;
    case IMAGE_FORMAT_ARGB32:
    case IMAGE_FORMAT_RGB32:
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_y);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->m->m_video_w, this->m->m_video_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->m->m_video_ptr);
        break;
    case IMAGE_FORMAT_RGB888:
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_y);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->m->m_video_w, this->m->m_video_h, 0, GL_RGB, GL_UNSIGNED_BYTE, this->m->m_video_ptr);
        break;
    case IMAGE_FORMAT_Grayscale8:
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_y);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, this->m->m_video_w, this->m->m_video_h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, this->m->m_video_ptr);
        break;
    case IMAGE_FORMAT_BGR888:
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, this->m->texture_id_y);
        f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->m->m_video_w, this->m->m_video_h, 0, GL_RGB, GL_UNSIGNED_BYTE, this->m->m_video_ptr);
        break;
    default:
        break;
    }
#endif
    f->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    f->glBindTexture(GL_TEXTURE_2D, 0);
    this->m->m_vao->release();
    this->m->m_program->release();
    f->glFinish();
}

void VideoOpenGL::paintEvent(QPaintEvent *e)
{
    QOpenGLWidget::paintEvent(e);
    QPainter painter(this);
    //painter.fillRect(0, 0, 100, 100, Qt::green);
}

void VideoOpenGL::set_image_size(const int &width, const int &height, const ImageFormat& format)
{
    this->m->m_video_w = width;
    this->m->m_video_h = height;
    this->m->format = format;

    float multiple = 1.0;
    switch (format)
    {
    case IMAGE_FORMAT_NV12:
    case IMAGE_FORMAT_NV21:
    case IMAGE_FORMAT_I420:
        multiple = 1.5;
        break;
    case IMAGE_FORMAT_ARGB32:
    case IMAGE_FORMAT_RGB32:
        multiple = 4;
        break;
    case IMAGE_FORMAT_RGB888:
    case IMAGE_FORMAT_BGR888:
        multiple = 3;
        break;
    case IMAGE_FORMAT_Grayscale8:
        multiple = 1;
        break;
    default:
        break;
    }
    this->m->m_image_size = multiple*this->m->m_video_w*this->m->m_video_h;

    if(this->m->m_video_ptr != nullptr){
        delete this->m->m_video_ptr;
        this->m->m_video_ptr = nullptr;
    }

    this->m->m_video_ptr = new char[this->m->m_image_size];
}

void VideoOpenGL::update_image(char *data_ptr)
{
    memcpy(this->m->m_video_ptr, data_ptr, this->m->m_image_size);
    update();
}
