//
// Created by dingrui on 2/1/26.
//

#include "x.h"

#include <QApplication>

#include "widget.h"

int main(int argc, char *argv[]) {
    XLog::Init();

    QApplication a(argc, argv);
    Widget w;
    w.show();

    return a.exec();
}
