cmake_minimum_required(VERSION 3.21)

# 输出详细日志
set(CMAKE_VERBOSE_MAKEFILE ON)

# 项目信息
project(x-video-player VERSION 1.0 LANGUAGES C CXX)

# 语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译告警
add_compile_options(
    -Wall
    -Werror
    -Wno-deprecated-declarations
)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# dep
set(LIB_DIRECTORY ${CMAKE_SOURCE_DIR}/cmake)
include(${LIB_DIRECTORY}/spdlog.cmake)
include(${LIB_DIRECTORY}/FFmpeg.cmake)
include(${LIB_DIRECTORY}/Qt.cmake)

# qt auto tools
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(LIB_NAME "${PROJECT_NAME}-core")

add_subdirectory(x_video_player)
add_subdirectory(app)
include(CTest)
enable_testing()
add_subdirectory(tests)