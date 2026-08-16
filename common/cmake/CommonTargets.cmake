# ============================================================
# CommonTargets.cmake — common/ 库 target 命名约定
#
# 约定（类比 Qt${QT_VERSION_MAJOR}::Core）：
#   - 实际 target：${COMMON_LIB_PREFIX}<basename>（默认 common_<basename>）
#   - 对外链接：${COMMON_TARGET_NAMESPACE}::<basename>（勿手写 common_ / ${COMMON_TARGET_NAMESPACE}::）
#   - 定义库：fz2_add_common_library(<basename> SHARED|STATIC ...)
#   - 判断存在：fz2_common_target_exists(<basename> <out_var>)
#
# common/plugins 下 Qt 业务插件（Core、Permission 等）保持 PLUGIN_NAME，不适用本约定。
# ============================================================

set(COMMON_LIB_PREFIX "common_" CACHE INTERNAL
    "common/ 下共享库 CMake target 前缀")

set(COMMON_TARGET_NAMESPACE "common" CACHE INTERNAL
    "common 库 CMake 别名命名空间；链接写法：${COMMON_TARGET_NAMESPACE}::<basename>")

# ── 规范化 basename（去掉已有前缀，避免 common_common_xxx）────────────────────
function(fz2_common_basename name out_var)
    string(REGEX REPLACE "^${COMMON_LIB_PREFIX}" "" _base "${name}")
    set(${out_var} "${_base}" PARENT_SCOPE)
endfunction()

# ── basename → 实际 target 名（${COMMON_LIB_PREFIX}<basename>）────────────────
function(fz2_common_target_name basename out_var)
    fz2_common_basename("${basename}" _base)
    set(${out_var} "${COMMON_LIB_PREFIX}${_base}" PARENT_SCOPE)
endfunction()

# ── basename → 链接用别名（${COMMON_TARGET_NAMESPACE}::<basename>）──────────
function(fz2_common_alias_name basename out_var)
    fz2_common_basename("${basename}" _base)
    set(${out_var} "${COMMON_TARGET_NAMESPACE}::${_base}" PARENT_SCOPE)
endfunction()

# ── 判断 common 库 target 是否已定义 ────────────────────────────────────────
function(fz2_common_target_exists basename out_var)
    fz2_common_target_name("${basename}" _tgt)
    if(TARGET ${_tgt})
        set(${out_var} TRUE PARENT_SCOPE)
    else()
        set(${out_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

# ── 创建共享/静态库 + 命名空间别名 ───────────────────────────────────────────
function(fz2_add_common_library basename type)
    fz2_common_target_name("${basename}" _tgt)
    fz2_common_alias_name("${basename}" _alias)

    add_library(${_tgt} ${type} ${ARGN})

    if(NOT TARGET ${_alias})
        add_library(${_alias} ALIAS ${_tgt})
    endif()

    set(FZ2_COMMON_LIBRARY_TARGET ${_tgt} PARENT_SCOPE)
    set(FZ2_COMMON_LIBRARY_BASENAME ${_alias} PARENT_SCOPE)
endfunction()

# ── 创建 INTERFACE 库 + 命名空间别名 ─────────────────────────────────────────
function(fz2_add_common_interface basename)
    fz2_common_target_name("${basename}" _tgt)
    fz2_common_alias_name("${basename}" _alias)

    add_library(${_tgt} INTERFACE)

    if(NOT TARGET ${_alias})
        add_library(${_alias} ALIAS ${_tgt})
    endif()

    set(FZ2_COMMON_LIBRARY_TARGET ${_tgt} PARENT_SCOPE)
    set(FZ2_COMMON_LIBRARY_BASENAME ${_alias} PARENT_SCOPE)
endfunction()
