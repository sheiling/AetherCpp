#include "mark_template_kind.h"

namespace aoi {

vision_algo::ModelProcKind toVisionModelKind(MarkTemplateKind kind)
{
    return kind == MarkTemplateKind::Chip ? vision_algo::ModelProcKind::Chip
                                          : vision_algo::ModelProcKind::Mark;
}

} // namespace aoi
