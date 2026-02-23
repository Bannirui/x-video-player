//
// Created by rui ding on 2025/9/12.
//

#include "x_video_player/x_log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> XLog::m_logger;

void XLog::Init()
{
    if (m_logger)
    {
        return;
    }
    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    m_logger = std::make_shared<spdlog::logger>("x-video-player", sink);
    m_logger->set_pattern("%^[%T] [%l] %n: %v%$");
    m_logger->set_level(spdlog::level::trace);
}
