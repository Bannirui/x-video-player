//
// Created by rui ding on 2025/9/12.
//

#include "x/x_log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> XLog::s_logger;

void XLog::Init()
{
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sink->set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    s_logger = std::make_shared<spdlog::logger>("X", sink);
    s_logger->set_level(spdlog::level::trace);
}
