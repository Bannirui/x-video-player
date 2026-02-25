//
// Created by dingrui on 2/1/26.
//

#include <QApplication>

#include "widget.h"

#include "../x_video_player/src/x/x_log.h"

int main(int argc, char *argv[]) {
    XLog::Init();

    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
