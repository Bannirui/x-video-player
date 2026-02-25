//
// Created by rui ding on 2025/9/12.
//

#pragma once

#include <spdlog/spdlog.h>

#define XLOG_INFO(...)  ::XLog::get_logger()->info(__VA_ARGS__)
#define XLOG_WARN(...)  ::XLog::get_logger()->warn(__VA_ARGS__)
#define XLOG_ERROR(...) ::XLog::get_logger()->error(__VA_ARGS__)

class XLog
{
public:
    static void Init();

    static std::shared_ptr<spdlog::logger> get_logger() { return s_logger; }

private:
    static std::shared_ptr<spdlog::logger> s_logger;
};
