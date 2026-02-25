//
// Created by dingrui on 2/1/26.
//

#include "x.h"

#include <QApplication>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QThread>
#include <QFile>
#include <QTimer>

#include "widget.h"

int main(int argc, char *argv[])
{
    XLog::Init();

    QApplication a(argc, argv);

    QAudioFormat fmt;
    fmt.setSampleRate(44100);
    fmt.setSampleSize(16);
    fmt.setChannelCount(2);
    fmt.setCodec("audio/pcm");
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::UnSignedInt);

    QAudioOutput *out = new QAudioOutput(fmt);
    QIODevice    *io  = out->start();  // start play

    QFile *file = new QFile("asset/Python.pcm");
    file->open(QIODevice::ReadOnly);

    QTimer *timer = new QTimer;

    QObject::connect(timer, &QTimer::timeout,
                     [=]()
                     {
                         if (out->bytesFree() > 0)
                         {
                             QByteArray data = file->read(out->periodSize());
                             if (data.isEmpty())
                             {
                                 timer->stop();
                                 file->close();
                                 return;
                             }
                             io->write(data);
                         }
                     });
    timer->start(1);

    // Widget w;
    // w.show();

    return a.exec();
}
