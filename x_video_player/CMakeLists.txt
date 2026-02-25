# 源文件
file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
set(HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
file(GLOB_RECURSE HDRS
        "${HDR_DIR}/*.h"
        "${HDR_DIR}/*.hpp"
)

# 添加头文件的目的仅仅是为了让IDEA识别到 新建文件的时候添加索引
add_library(${LIB_NAME} STATIC
        ${HDRS}
        ${SRCS}
)

target_include_directories(${LIB_NAME}
    PUBLIC
        ${HDR_DIR}
        ${FFMPEG_INCLUDE_DIRS}
)

# compile flags
target_compile_options(${LIB_NAME}
    PRIVATE ${FFMPEG_CFLAGS_OTHER}
)

target_link_libraries(${LIB_NAME}
    PUBLIC
        spdlog
        ${FFMPEG_LIBRARIES}
        Qt${QT_VERSION_MAJOR}::Core
)

target_precompile_headers(${LIB_NAME}
    PRIVATE ${HDR_DIR}/pch.h
)
