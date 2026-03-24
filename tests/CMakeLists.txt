# 拷贝资源文件到构建目录和运行目录
set(ASSET_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/asset")
set(ASSET_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/asset")
set(ASSET_RUN_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/asset")

if (EXISTS "${ASSET_SRC_DIR}")
    add_custom_target(test_assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSET_SRC_DIR}" "${ASSET_BIN_DIR}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ASSET_SRC_DIR}" "${ASSET_RUN_DIR}"
            COMMENT "Copying assets to build and runtime directories"
    )
endif ()

# Collect all cpp files in this directory
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

foreach (TEST_SRC ${TEST_SOURCES})
    # Extract file name without extension
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    # Create executable
    add_executable(${TEST_NAME} ${TEST_SRC})
    # ffmpeg用cpp编译时可能会缺少宏 添加宏时不要污染全局
    target_compile_definitions(${TEST_NAME} PRIVATE __STDC_CONSTANT_MACROS)
    # Link main library
    target_link_libraries(${TEST_NAME}
            PRIVATE ${LIB_NAME}
    )
    # 构建时触发资源文件复制
    if (TARGET test_assets)
        add_dependencies(${TEST_NAME} test_assets)
    endif ()
    # Register with CTest
    add_test(NAME ${TEST_NAME}
            COMMAND ${TEST_NAME}
    )
endforeach ()
