//
// Created by dingrui on 2/1/26.
//

#include <iostream>

extern "C" {
#include <libavutil/avutil.h>
}

int main(int argc, char **argv) {
    std::cout << "FFmpeg: " << av_version_info() << std::endl;
    return 0;
}
