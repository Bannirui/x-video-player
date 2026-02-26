//
// Created by dingrui on 2/1/26.
//

#include "x.h"

#include <QApplication>
#include <QSurfaceFormat>

#include "widget.h"

int main(int argc, char *argv[])
{
    XLog::Init();

    QSurfaceFormat format;
    format.setVersion(4, 5);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication a(argc, argv);

    Widget w;
    w.show();

    return a.exec();
}
