//
// Created by dingrui on 4/18/26.
//

#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <memory>

// 自定义删除器
inline auto avcodec_parameters_deleter = [](AVCodecParameters* p) -> void {
    if (p) avcodec_parameters_free(&p);
};
// 使用自定义删除器的智能指针类型
using AVCodecParametersPtr = std::unique_ptr<AVCodecParameters, decltype(avcodec_parameters_deleter)>;
