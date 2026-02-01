# 源文件
file(GLOB_RECURSE SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
set(HDR_DIR ${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE HDRS
        "${HDR_DIR}/*.h"
        "${HDR_DIR}/*.hpp"
)

# 添加头文件的目的仅仅是为了让IDEA识别到 新建文件的时候添加索引
add_library(${LIB_NAME} STATIC ${SRCS} ${HDRS})

target_include_directories(${LIB_NAME}
    PUBLIC
        ${HDR_DIR}
        ${FFMPEG_INCLUDE_DIRS}
)

# compile flags
target_compile_options(${LIB_NAME}
    PRIVATE
        ${FFMPEG_CFLAGS_OTHER}
)

target_link_libraries(${LIB_NAME}
    PUBLIC
        spdlog
        ${FFMPEG_LIBRARIES}
)
