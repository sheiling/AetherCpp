# ============================================================
# FZ2Helpers.cmake — 项目级通用 CMake 辅助函数
#
# 使用方式（在 common/CMakeLists.txt 中）：
#   list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#   include(FZ2Helpers)
# ============================================================

include(CommonTargets)

# 本文件所在目录（CACHE 供 app/plugins 等兄弟目录在函数内引用同目录脚本）
set(FZ2_CMAKE_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL
    "FZ2Helpers.cmake 所在目录")

# ══════════════════════════════════════════════════════════════
# fz2_setup_target(target vs_folder)
#
#   设置 exe / 共享库的输出目录：
#     RUNTIME / LIBRARY → bin/<Config>/
#     ARCHIVE           → lib/<Config>/
#   并将 target 归入 VS 解决方案文件夹 vs_folder。
# ══════════════════════════════════════════════════════════════
function(fz2_setup_target target vs_folder)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release"
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/lib/Debug"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/lib/Release"
        ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/lib/RelWithDebInfo"
        FOLDER "${vs_folder}"
    )
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_setup_plugin(target vs_folder)
#
#   设置插件 DLL 的输出目录：
#     RUNTIME / LIBRARY → bin/<Config>/plugins/
#     ARCHIVE           → lib/<Config>/
#   PREFIX 置空（避免 libXxx.dll），并归入 VS 文件夹 vs_folder。
# ══════════════════════════════════════════════════════════════
function(fz2_setup_plugin target vs_folder)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug/plugins"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release/plugins"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/plugins"
        LIBRARY_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/bin/Debug/plugins"
        LIBRARY_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/bin/Release/plugins"
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo/plugins"
        ARCHIVE_OUTPUT_DIRECTORY_DEBUG          "${CMAKE_BINARY_DIR}/lib/Debug"
        ARCHIVE_OUTPUT_DIRECTORY_RELEASE        "${CMAKE_BINARY_DIR}/lib/Release"
        ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/lib/RelWithDebInfo"
        PREFIX ""
        FOLDER "${vs_folder}"
    )
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_remove_disabled_plugin_artifacts(<stem> [<stem> ...])
#
#   插件未参与本次配置时，删除 bin/<Config>/plugins/ 下残留 DLL、PDB。
#   stem 为插件 DLL 文件名（不含 .dll），如 Camera、LightenDetector。
# ══════════════════════════════════════════════════════════════
function(fz2_remove_disabled_plugin_artifacts)
    if(ARGC EQUAL 0)
        return()
    endif()

    foreach(_stem IN LISTS ARGN)
        foreach(_config IN ITEMS Debug Release RelWithDebInfo)
            set(_plugin_dir "${CMAKE_BINARY_DIR}/bin/${_config}/plugins")
            foreach(_ext IN ITEMS dll pdb)
                set(_artifact "${_plugin_dir}/${_stem}.${_ext}")
                if(EXISTS "${_artifact}")
                    file(REMOVE "${_artifact}")
                    message(STATUS "  [fz2] 已删除未启用插件产物: ${_artifact}")
                endif()
            endforeach()
        endforeach()
    endforeach()
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_collect_sources(out_sources out_headers [extra_exclude_regex ...])
#
#   在 CMAKE_CURRENT_SOURCE_DIR 下递归收集：
#     out_sources ← *.cpp、*.cc
#     out_headers ← *.h、*.hpp
#
#   默认排除（不参与库/插件目标编译与 IDE 分组）：
#     examples/、example/、tests/、test/
#   可传入附加正则（与默认规则一并 list(FILTER EXCLUDE REGEX)）。
#
#   CONFIGURE_DEPENDS：源文件增减时触发 CMake 重新配置。
#
#   注意：不适用于同一 target 内按编译选项增减 .cpp 的模块（运控驱动、算法单元等）。
#   此类场景请维护显式编译列表，并用 fz2_expand_tree_files 生成 IDE 树文件列表。
# ══════════════════════════════════════════════════════════════
function(fz2_collect_sources out_sources out_headers)
    file(GLOB_RECURSE _srcs
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cc"
    )
    file(GLOB_RECURSE _hdrs
        CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    )

    set(_fz2_exclude_patterns
        ".*/examples/.*"
        ".*/example/.*"
        ".*/tests/.*"
        ".*/test/.*"
    )
    foreach(_pat IN LISTS _fz2_exclude_patterns ARGN)
        list(FILTER _srcs EXCLUDE REGEX "${_pat}")
        list(FILTER _hdrs EXCLUDE REGEX "${_pat}")
    endforeach()

    set(_rel_srcs)
    set(_rel_hdrs)
    foreach(_s IN LISTS _srcs)
        file(RELATIVE_PATH _rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_s}")
        list(APPEND _rel_srcs "${_rel}")
    endforeach()
    foreach(_h IN LISTS _hdrs)
        file(RELATIVE_PATH _rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_h}")
        list(APPEND _rel_hdrs "${_rel}")
    endforeach()

    set(${out_sources} ${_rel_srcs} PARENT_SCOPE)
    set(${out_headers} ${_rel_hdrs} PARENT_SCOPE)
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_expand_tree_files(out_files SOURCES src... [EXTRA file...])
#
#   由「实际参与编译的源文件列表」展开 VS 树 / IDE 文件列表：
#     - 保留 SOURCES、EXTRA 中的每一项；
#     - 对每个 .cpp/.cc 在同目录查找同名 .h/.hpp 并追加。
#   路径相对于 CMAKE_CURRENT_SOURCE_DIR（与手动维护的编译列表一致）。
# ══════════════════════════════════════════════════════════════
function(fz2_expand_tree_files out_files)
    cmake_parse_arguments(_fz2_et "" "" "SOURCES;EXTRA" ${ARGN})
    set(_files)
    foreach(_item IN LISTS _fz2_et_SOURCES _fz2_et_EXTRA)
        list(APPEND _files "${_item}")
    endforeach()

    foreach(_src IN LISTS _fz2_et_SOURCES)
        if(_src MATCHES "\\.(cpp|cc)$")
            get_filename_component(_dir "${_src}" DIRECTORY)
            get_filename_component(_base "${_src}" NAME_WE)
            if(_dir STREQUAL "" OR _dir STREQUAL ".")
                set(_base_dir "${CMAKE_CURRENT_SOURCE_DIR}")
                set(_rel_prefix "")
            else()
                set(_base_dir "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}")
                set(_rel_prefix "${_dir}/")
            endif()
            foreach(_ext h hpp)
                if(EXISTS "${_base_dir}/${_base}.${_ext}")
                    list(APPEND _files "${_rel_prefix}${_base}.${_ext}")
                endif()
            endforeach()
        endif()
    endforeach()

    list(REMOVE_DUPLICATES _files)
    set(${out_files} ${_files} PARENT_SCOPE)
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_glob_relative_append(out_var pattern [pattern ...])
#
#   将 GLOB 命中文件转为相对 CMAKE_CURRENT_SOURCE_DIR 的路径后追加到 out_var。
# ══════════════════════════════════════════════════════════════
function(fz2_glob_relative_append out_var)
    set(_acc ${${out_var}})
    foreach(_pattern ${ARGN})
        file(GLOB _hits CONFIGURE_DEPENDS ${_pattern})
        foreach(_hit IN LISTS _hits)
            file(RELATIVE_PATH _rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_hit}")
            list(APPEND _acc "${_rel}")
        endforeach()
    endforeach()
    list(REMOVE_DUPLICATES _acc)
    set(${out_var} ${_acc} PARENT_SCOPE)
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_source_group_tree(prefix [ROOT dir] FILES file1 file2 ...)
#
#   VS 解决方案资源管理器：按目录树展示源文件（对齐 scheduler 模块）。
#   ROOT 默认为 CMAKE_CURRENT_SOURCE_DIR；PREFIX 为顶层虚拟文件夹名。
#   FILES 须与 add_library 中路径一致；跨 ROOT 的第三方源（如 serial 3rdparty）单独调用一次。
# ══════════════════════════════════════════════════════════════
function(fz2_source_group_tree prefix)
    cmake_parse_arguments(_fz2_sg "" "ROOT" "FILES" ${ARGN})
    if(NOT _fz2_sg_ROOT)
        set(_fz2_sg_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    if(_fz2_sg_FILES)
        source_group(TREE "${_fz2_sg_ROOT}" PREFIX "${prefix}" FILES ${_fz2_sg_FILES})
    endif()
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_target_source_group_tree(target prefix [ROOT dir] FILES ...)
#
#   在 fz2_source_group_tree 基础上，将 FILES 中的 .h/.hpp 登记到 target（仅 IDE 展示），
#   确保 VS 过滤器对「仅头文件」亦生效（source_group 只作用于 target 已登记的源）。
# ══════════════════════════════════════════════════════════════
function(fz2_target_source_group_tree target prefix)
    cmake_parse_arguments(_fz2_tsg "" "ROOT" "FILES" ${ARGN})
    set(_headers)
    foreach(_f IN LISTS _fz2_tsg_FILES)
        if(_f MATCHES "\\.(h|hpp)$")
            list(APPEND _headers "${_f}")
        endif()
    endforeach()
    if(_headers)
        target_sources(${target} PRIVATE ${_headers})
    endif()
    if(_fz2_tsg_ROOT)
        fz2_source_group_tree("${prefix}" ROOT "${_fz2_tsg_ROOT}" FILES ${_fz2_tsg_FILES})
    else()
        fz2_source_group_tree("${prefix}" FILES ${_fz2_tsg_FILES})
    endif()
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_target_msvc_options(target)
#
#   在 MSVC 下为单个 target 追加与根 CMakeLists 全局一致的编译选项。
#   根目录已 add_compile_options(/wd4251 等)，全工程 target 默认已屏蔽；
#   本函数保留给需显式声明、或 configure 早于全局选项的特殊 target。
#   各 /wd 含义见根 CMakeLists.txt「MSVC 全局选项」注释。
# ══════════════════════════════════════════════════════════════
function(fz2_target_msvc_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE
            /utf-8 /W3
            /wd4251 /wd4275
            /wd4267 /wd4244 /wd4996
            /wd4828 /wd5208
            /wd4819
        )
    endif()
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_target_compile_translations(target source_dir output_dir ts_files)
#
#   构建后将指定目录下的 .ts 编译为 .qm，输出到 output_dir（不写入源码树）。
#   ts_files：分号分隔的 .ts 文件名列表。
#   依赖 Qt${QT_VERSION_MAJOR}::qmake 同目录下的 lrelease；未找到时仅 WARNING。
# ══════════════════════════════════════════════════════════════
function(fz2_target_compile_translations target source_dir output_dir ts_files)
    if(NOT target OR NOT source_dir OR NOT output_dir OR NOT ts_files)
        message(FATAL_ERROR "fz2_target_compile_translations: 需要 target、source_dir、output_dir 与 ts_files")
    endif()

    get_target_property(_qmake_exe Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    if(NOT _qmake_exe)
        message(WARNING "[fz2] 未找到 qmake，跳过 '${target}' 的翻译编译")
        return()
    endif()

    get_filename_component(_qt_bin_dir "${_qmake_exe}" DIRECTORY)
    find_program(_fz2_lrelease_executable lrelease
        HINTS "${_qt_bin_dir}"
        NO_DEFAULT_PATH
    )

    if(NOT _fz2_lrelease_executable)
        message(WARNING "[fz2] 未找到 lrelease，跳过 '${target}' 的翻译编译")
        return()
    endif()

    # MSBuild 将 ; 视为命令分隔符，列表改用 | 传入脚本再还原
    string(REPLACE ";" "|" _ts_files_pipe "${ts_files}")

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${output_dir}"
        COMMAND "${CMAKE_COMMAND}"
            -DLRELEASE_EXECUTABLE=${_fz2_lrelease_executable}
            -DSOURCE_DIR=${source_dir}
            -DOUTPUT_DIR=${output_dir}
            -DTS_FILES=${_ts_files_pipe}
            -P "${FZ2_CMAKE_MODULE_DIR}/CompileTranslations.cmake"
        COMMENT "[fz2] 编译翻译文件 ${target} (.ts -> .qm)..."
        VERBATIM
    )
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_target_compile_app_translations(target source_config_dir ts_files)
#
#   主程序翻译：输出到可执行文件同目录下的 config/。
# ══════════════════════════════════════════════════════════════
function(fz2_target_compile_app_translations target source_config_dir ts_files)
    set(_output_dir "$<TARGET_FILE_DIR:${target}>/config")
    fz2_target_compile_translations(${target} "${source_config_dir}" "${_output_dir}" "${ts_files}")
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_target_compile_plugin_translations(target source_i18n_dir ts_files)
#
#   插件翻译：输出到 ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/i18n/。
# ══════════════════════════════════════════════════════════════
function(fz2_target_compile_plugin_translations target source_i18n_dir ts_files)
    set(_i18n_output_dir "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/config/i18n")
    fz2_target_compile_translations(${target} "${source_i18n_dir}" "${_i18n_output_dir}" "${ts_files}")
endfunction()

# ══════════════════════════════════════════════════════════════
# Qt-Advanced-Stylesheets 集成
# 源码位于 common/3rdparty/Qt-Advanced-Stylesheets，直接编译进目标库
#
# 文件作用域：CMAKE_CURRENT_LIST_DIR 指向本文件所在目录（common/cmake/）
# 将 common/ 根目录存入全局属性，供函数内安全读取
# ══════════════════════════════════════════════════════════════
get_filename_component(_FZ2_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set_property(GLOBAL PROPERTY _FZ2_COMMON_DIR "${_FZ2_COMMON_DIR}")

set(ACSS_SOURCE_DIR "${_FZ2_COMMON_DIR}/3rdparty/Qt-Advanced-Stylesheets/src")
set(ACSS_SOURCES "")
set(ACSS_HEADERS "")

if(EXISTS "${ACSS_SOURCE_DIR}/QtAdvancedStylesheet.h")
    # 仅包含 Widget 部分，不含 QML 相关文件
    set(ACSS_SOURCES
        "${ACSS_SOURCE_DIR}/QtAdvancedStylesheet.cpp"
    )
    set(ACSS_HEADERS
        "${ACSS_SOURCE_DIR}/acss_globals.h"
        "${ACSS_SOURCE_DIR}/QtAdvancedStylesheet.h"
    )
    message(STATUS "  [ACSS] Qt-Advanced-Stylesheets found: ${ACSS_SOURCE_DIR}")
else()
    message(WARNING "[ACSS] Qt-Advanced-Stylesheets source not found: ${ACSS_SOURCE_DIR}")
    message(WARNING "[ACSS] Please place Qt-Advanced-Stylesheets in common/3rdparty/Qt-Advanced-Stylesheets")
endif()

# ══════════════════════════════════════════════════════════════
# fz2_target_link_acss(target)
#
#   将 Qt-Advanced-Stylesheets 源码编译进指定目标，并添加必要的
#   包含路径和编译定义（ACSS_STATIC）。
#   需在 add_library / add_executable 之后调用。
# ══════════════════════════════════════════════════════════════
function(fz2_target_link_acss target)
    get_property(_common_dir GLOBAL PROPERTY _FZ2_COMMON_DIR)
    set(_acss_src_dir "${_common_dir}/3rdparty/Qt-Advanced-Stylesheets/src")

    if(NOT EXISTS "${_acss_src_dir}/QtAdvancedStylesheet.h")
        message(WARNING "[ACSS] fz2_target_link_acss: source not found, skipping target '${target}'")
        return()
    endif()

    # 仅编译 Widget 部分，不包含 QML 相关文件
    target_sources(${target} PRIVATE
        "${_acss_src_dir}/QtAdvancedStylesheet.cpp"
        "${_acss_src_dir}/acss_globals.h"
        "${_acss_src_dir}/QtAdvancedStylesheet.h"
    )
    target_include_directories(${target} PUBLIC
        $<BUILD_INTERFACE:${_acss_src_dir}>
    )
    target_compile_definitions(${target} PUBLIC ACSS_STATIC)
endfunction()

# ══════════════════════════════════════════════════════════════
# fz2_copy_acss_styles(target)
#
#   在构建后将 Qt-Advanced-Stylesheets/styles 目录复制到可执行文件
#   输出目录下的 styles/ 子目录，确保运行时能找到主题文件。
#   需在 add_library / add_executable 之后调用。
# ══════════════════════════════════════════════════════════════
function(fz2_copy_acss_styles target)
    get_property(_common_dir GLOBAL PROPERTY _FZ2_COMMON_DIR)
    set(_styles_src "${_common_dir}/3rdparty/Qt-Advanced-Stylesheets/styles")

    if(NOT EXISTS "${_styles_src}")
        message(WARNING "[ACSS] fz2_copy_acss_styles: styles directory not found: ${_styles_src}")
        return()
    endif()

    get_target_property(_target_type ${target} TYPE)

    if(_target_type STREQUAL "EXECUTABLE")
        set(_styles_dst "$<TARGET_FILE_DIR:${target}>/styles")
    else()
        get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
        if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
            if(_is_multi_config)
                set(_styles_dst "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/styles")
            else()
                set(_styles_dst "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/styles")
            endif()
        else()
            if(_is_multi_config)
                set(_styles_dst "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/styles")
            else()
                set(_styles_dst "${CMAKE_BINARY_DIR}/bin/styles")
            endif()
        endif()
    endif()

    message(STATUS "  [ACSS] Will copy styles: ${_styles_src} → ${_styles_dst}")

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_styles_dst}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${_styles_src}" "${_styles_dst}"
        COMMENT "[ACSS] Copying Qt-Advanced-Stylesheets styles..."
        VERBATIM
    )
endfunction()
