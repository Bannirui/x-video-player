//
// Created by dingrui on 2/27/26.
//

#include "x/x_log.h"
#include "x/demux.h"

int main()
{
    XLog::Init();

    Demux       demux;
    std::string path = "asset/Python.mp4";
    bool        ret  = demux.Open(path);
    XLOG_INFO("ret: {0}", ret);
    return 0;
}