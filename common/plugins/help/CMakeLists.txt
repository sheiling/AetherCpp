# ============================================================
# HelpPlugin — 帮助页（版本信息 + PDF 手册，内嵌 MuPDF 渲染）
# ============================================================

set(PLUGIN_NAME Help)

set(MUPDF_ROOT "${PROJECT_SOURCE_DIR}/common/3rdparty/mupdf-1.27.2")
set(MUPDF_INCLUDE "${MUPDF_ROOT}/include")
set(MUPDF_LIB_ROOT "${MUPDF_ROOT}/lib/x64")
set(MUPDF_COMPAT_INCLUDE "${CMAKE_CURRENT_BINARY_DIR}/mupdf_include_compat")

# 第三方头文件使用 #include "mupdf/..."，但 Aether 包内为扁平 include/ 布局
set(_MUPDF_COMPAT_MARKER "${MUPDF_COMPAT_INCLUDE}/mupdf/fitz/version.h")
if(NOT EXISTS "${_MUPDF_COMPAT_MARKER}")
    file(MAKE_DIRECTORY "${MUPDF_COMPAT_INCLUDE}/mupdf")
    foreach(_MUPDF_ENTRY IN ITEMS fitz fitz.h pdf pdf.h memento.h html.h ucdn.h helpers)
        if(EXISTS "${MUPDF_INCLUDE}/${_MUPDF_ENTRY}")
            file(COPY "${MUPDF_INCLUDE}/${_MUPDF_ENTRY}"
                 DESTINATION "${MUPDF_COMPAT_INCLUDE}/mupdf")
        endif()
    endforeach()
endif()
unset(_MUPDF_COMPAT_MARKER)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

fz2_collect_sources(PLUGIN_SOURCES PLUGIN_HEADERS)

add_library(${PLUGIN_NAME} SHARED
    ${PLUGIN_SOURCES}
    ${PLUGIN_HEADERS}
    helpplugin.json
)

target_compile_features(${PLUGIN_NAME} PRIVATE cxx_std_17)

target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/view
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/pdf
    ${MUPDF_COMPAT_INCLUDE}
    ${PROJECT_SOURCE_DIR}/common/include
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    ${COMMON_TARGET_NAMESPACE}::extensionsystem
    ${COMMON_TARGET_NAMESPACE}::messagecenter
    ${COMMON_TARGET_NAMESPACE}::uibase
    ${COMMON_TARGET_NAMESPACE}::config
)

set(_MUPDF_LIB_DIR "${MUPDF_LIB_ROOT}/$<IF:$<CONFIG:Debug>,debug,release>")
target_link_libraries(${PLUGIN_NAME} PRIVATE
    "${_MUPDF_LIB_DIR}/libmupdf.lib"
    "${_MUPDF_LIB_DIR}/libthirdparty.lib"
    "${_MUPDF_LIB_DIR}/libpkcs7.lib"
)
unset(_MUPDF_LIB_DIR)

fz2_target_msvc_options(${PLUGIN_NAME})

fz2_setup_plugin(${PLUGIN_NAME} "common/plugins/help")

fz2_source_group_tree(help FILES ${PLUGIN_SOURCES} ${PLUGIN_HEADERS})

message(STATUS "  [HelpPlugin] Plugin target configured.")
