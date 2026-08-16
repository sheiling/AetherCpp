#include "display_image_utils.h"

#include <algorithm>

namespace camera_plugin {

QImage downscaleForDisplay(const QImage &source, int maxEdge)
{
    if (source.isNull() || maxEdge <= 0) {
        return source;
    }

    const int w = source.width();
    const int h = source.height();
    const int longEdge = std::max(w, h);
    if (longEdge <= maxEdge) {
        return source;
    }

    const double scale = static_cast<double>(maxEdge) / static_cast<double>(longEdge);
    // FastTransformation：角色预览重实时性，非画质优先
    return source.scaled(static_cast<int>(w * scale), static_cast<int>(h * scale),
                         Qt::KeepAspectRatio, Qt::FastTransformation);
}

} // namespace camera_plugin
