//
// Created by dingrui on 2/26/26.
//

#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    XVideoWidget(QWidget *parent = nullptr);
    ~XVideoWidget();

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    QOpenGLShaderProgram m_program;

    QOpenGLBuffer            m_vbo;
    QOpenGLVertexArrayObject m_vao;

    // uniform location, yuv
    GLuint m_uLocations[3] = {0};
    // OpenGL texture id, yuv
    GLuint m_textures[3] = {0};

    int m_width{240};
    int m_height{128};

    unsigned char *m_datas[3] = {0};
};
