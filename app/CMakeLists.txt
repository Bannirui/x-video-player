set(HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE HDRS CONFIGURE_DEPENDS
        "${HDR_DIR}/*.h"
        "${HDR_DIR}/*.hpp"
)
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.cpp"
)
file(GLOB_RECURSE UIS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui)
file(GLOB RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.qrc)

# 拷贝资源文件到构建目录和运行目录
set(ASSET_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/asset")
set(ASSET_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/asset")
set(ASSET_RUN_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/asset")

if (EXISTS "${ASSET_SRC_DIR}")
    add_custom_target(copy_assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSET_SRC_DIR}" "${ASSET_BIN_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSET_SRC_DIR}" "${ASSET_RUN_DIR}"
            COMMENT "Copying assets to build and runtime directories"
    )
endif ()

add_executable(${PROJECT_NAME}
        ${HDRS}
        ${SRCS}
        ${UIS}
        ${RESOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${HDR_DIR}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Qt${QT_VERSION_MAJOR}::Widgets
            ${LIB_NAME}
)

# 构建时触发资源文件复制
if (TARGET copy_assets)
    add_dependencies(${PROJECT_NAME} copy_assets)
endif ()