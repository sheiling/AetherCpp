# =============================================================================
# VisionAlgoUnits.cmake — 算法单元编译开关与依赖自动传播
#
# 单元依赖关系（开启子单元时自动开启父依赖）：
#   ModelProcessor  ← MarkPointLocate / ChipDetector / DieBondingChipDetect
#   CISDK detection libs ← 上述检测单元
#   ZXing            ← BarcodeScan（独立，不依赖 CISDK 检测）
#   dl_die_bonding_detector ← DieBondingChipDetect + VISION_ALGO_ENABLE_DL_DIE_BONDING
# =============================================================================

option(VISION_ALGO_ENABLE_BARCODE
    "Enable BarcodeScan unit (ZXing-cpp)" ON)
option(VISION_ALGO_ENABLE_MODEL
    "Enable ModelProcessor / CISDK model_processor (template load & make)" ON)
option(VISION_ALGO_ENABLE_MARKER
    "Enable MarkPointLocate unit (marker_detector + model)" ON)
option(VISION_ALGO_ENABLE_CHIP
    "Enable ChipDetector unit (chip_detector + model)" OFF)
option(VISION_ALGO_ENABLE_DIE_BONDING
    "Enable DieBondingChipDetect unit (traditional + optional DL)" ON)
option(VISION_ALGO_ENABLE_DL_DIE_BONDING
    "Enable DL backend for DieBondingChipDetect (dl_die_bonding_detector)" ON)

# --- 依赖传播：检测类算法均依赖模板加载 ---
if(VISION_ALGO_ENABLE_MARKER OR VISION_ALGO_ENABLE_CHIP OR VISION_ALGO_ENABLE_DIE_BONDING)
    if(NOT VISION_ALGO_ENABLE_MODEL)
        message(STATUS "[VisionAlgo] auto-enable MODEL (required by MARKER/CHIP/DIE_BONDING)")
        set(VISION_ALGO_ENABLE_MODEL ON CACHE BOOL "Enable ModelProcessor" FORCE)
    endif()
endif()

# DL 固晶依赖 DieBonding 总开关
if(VISION_ALGO_ENABLE_DL_DIE_BONDING AND NOT VISION_ALGO_ENABLE_DIE_BONDING)
    message(STATUS "[VisionAlgo] auto-enable DIE_BONDING (required by DL_DIE_BONDING)")
    set(VISION_ALGO_ENABLE_DIE_BONDING ON CACHE BOOL "Enable DieBondingChipDetect" FORCE)
endif()

# 汇总编译期宏，供源码 #ifdef 与链接方（如 Aoi）使用
set(VISION_ALGO_COMPILE_DEFS "")

macro(_vision_algo_add_def _name)
    list(APPEND VISION_ALGO_COMPILE_DEFS ${_name})
endmacro()

if(VISION_ALGO_ENABLE_BARCODE)
    _vision_algo_add_def(VISION_ALGO_HAS_BARCODE)
endif()
if(VISION_ALGO_ENABLE_MODEL)
    _vision_algo_add_def(VISION_ALGO_HAS_MODEL)
endif()
if(VISION_ALGO_ENABLE_MARKER)
    _vision_algo_add_def(VISION_ALGO_HAS_MARKER)
endif()
if(VISION_ALGO_ENABLE_CHIP)
    _vision_algo_add_def(VISION_ALGO_HAS_CHIP)
endif()
if(VISION_ALGO_ENABLE_DIE_BONDING)
    _vision_algo_add_def(VISION_ALGO_HAS_DIE_BONDING)
endif()
if(VISION_ALGO_ENABLE_DL_DIE_BONDING)
    _vision_algo_add_def(VISION_ALGO_HAS_DL_DIE_BONDING)
endif()

message(STATUS "[VisionAlgo] units: BARCODE=${VISION_ALGO_ENABLE_BARCODE} MODEL=${VISION_ALGO_ENABLE_MODEL}"
    " MARKER=${VISION_ALGO_ENABLE_MARKER} CHIP=${VISION_ALGO_ENABLE_CHIP}"
    " DIE_BONDING=${VISION_ALGO_ENABLE_DIE_BONDING} DL=${VISION_ALGO_ENABLE_DL_DIE_BONDING}")
