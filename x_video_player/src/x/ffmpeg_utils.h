//
// Created by dingrui on 4/18/26.
//

#pragma once

#include "x_log.h"

extern "C" {
#include <libavutil/error.h>
}
#include <string>

/**
 * 错误码转字符串
 * @param errNum ffmpeg的错误码
 */
inline std::string avErrToStr(int errNum) {
    char buf[256] = {0};
    av_strerror(errNum, buf, sizeof(buf));
    return std::string(buf);
}

// 日志辅助
inline void LogAvErr(int errNum, const std::string& msg) {
    XLOG_ERROR("{}: {}", msg, avErrToStr(errNum));
}