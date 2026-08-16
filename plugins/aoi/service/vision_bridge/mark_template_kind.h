#pragma once

/**
 * @file mark_template_kind.h
 * @brief Mark 模板类型（UI 展示用，与 CISDK ModelProcType 序号对齐）
 */

#include <vision_algo/units/model/model_processor_data.h>

namespace aoi {

enum class MarkTemplateKind : int
{
    Mark      = 1,
    Chip      = 2,
    Die       = 3,
    Solder    = 4,
    Socket    = 5,
    MicroCell = 6,
};

/** 映射到 VisionAlgo 支持的模板制作类型（当前仅 Mark / Chip） */
vision_algo::ModelProcKind toVisionModelKind(MarkTemplateKind kind);

} // namespace aoi
