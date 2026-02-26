//
// Created by dingrui on 2/26/26.
//

#include "x_video_widget.h"

#include <QDebug>
#include <QTimer>

// add "" for string, x=>"x"
#define GLSL_VERSION "#version 450 core\n"
#define GLSL(src) GLSL_VERSION #src

// clang-format off
const char *kVertexShader = GLSL(
    layout(location = 0) in vec3 a_pos;
    layout(location = 1) in vec2 a_uv;
    out vec2 v_uv;
    void main() {
        gl_Position = vec4(a_pos, 1.0);
        v_uv = a_uv;
    }
);

const char *kFragmentShader = GLSL(
    in vec2 v_uv;
    out vec4 FragColor;
    uniform sampler2D u_texY;
    uniform sampler2D u_texU;
    uniform sampler2D u_texV;
    void main() {
        vec3 yuv;
        vec3 rgb;
        yuv.x        = texture(u_texY, v_uv).r;
        yuv.y        = texture(u_texU, v_uv).r - 0.5;
        yuv.z        = texture(u_texV, v_uv).r - 0.5;
        rgb          = mat3(
            1.0, 1.0, 1.0,
            0.0, -0.39465, 2.03211,
            1.13983, -0.58060, 0.0
        ) * yuv;
        FragColor = vec4(rgb, 1.0);
    }
);
// clang-format on

#define A_VER 0  // vertex pos location
#define T_VER 1  // texture uv location

FILE *fp;

XVideoWidget::XVideoWidget(QWidget *parent) : QOpenGLWidget(parent), m_vbo(QOpenGLBuffer::VertexBuffer) {}

XVideoWidget::~XVideoWidget() {}

void XVideoWidget::initializeGL()
{
    initializeOpenGLFunctions();
    // compile shader
    if (!m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexShader))
    {
        qDebug() << m_program.log();
    }
    if (m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragmentShader))
    {
        qDebug() << m_program.log();
    }
    // compiler shader
    if (!m_program.link())
    {
        qDebug() << m_program.log();
    }
    m_program.bind();

    // vertex data
    // clang-format off
    static const GLfloat g_vertices[] = {
        // pos(xyz)         // uv(xy)
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
    };
    // clang-format on

    // VBO
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(g_vertices, sizeof(g_vertices));

    // VAO
    m_vao.create();
    m_vao.bind();

    // layout
    // pos(xyz)
    glVertexAttribPointer(A_VER, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(A_VER);
    // uv(xy)
    glVertexAttribPointer(T_VER, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(T_VER);

    m_vbo.release();
    m_vao.release();

    // uniform location
    m_uLocations[0] = m_program.uniformLocation("u_texY");
    m_uLocations[1] = m_program.uniformLocation("u_texU");
    m_uLocations[2] = m_program.uniformLocation("u_texV");

    // gen texture
    glGenTextures(3, m_textures);

    // YUV
    for (int i = 0; i < 3; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // allocate GPU memory buffer
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width / 2, m_height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, m_textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_width / 2, m_height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

    // CPU buffer
    m_datas[0] = new unsigned char[m_width * m_height];      // Y
    m_datas[1] = new unsigned char[m_width * m_height / 4];  // U
    m_datas[2] = new unsigned char[m_width * m_height / 4];  // V

    const char *filePath = "asset/Python.yuv";
    fp                   = fopen(filePath, "rb");
    if (!fp)
    {
        qDebug() << "Open file failed: " << filePath;
    }
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(40);
}

void XVideoWidget::paintGL()
{
    if (feof(fp))
    {
        fseek(fp, 0, SEEK_SET);
    }
    fread(m_datas[0], 1, m_width * m_height, fp);
    fread(m_datas[1], 1, m_width * m_height / 4, fp);
    fread(m_datas[2], 1, m_width * m_height / 4, fp);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, m_textures[0]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RED, GL_UNSIGNED_BYTE, m_datas[0]);
    glUniform1i(m_uLocations[0], 0);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, m_textures[1]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width / 2, m_height / 2, GL_RED, GL_UNSIGNED_BYTE, m_datas[1]);
    glUniform1i(m_uLocations[1], 1);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, m_textures[2]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width / 2, m_height / 2, GL_RED, GL_UNSIGNED_BYTE, m_datas[2]);
    glUniform1i(m_uLocations[2], 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_vao.release();
}

void XVideoWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}
