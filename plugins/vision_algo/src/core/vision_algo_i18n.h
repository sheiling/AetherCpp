#pragma once

/**
 * @file vision_algo_i18n.h
 * @brief VisionAlgo 插件内部文案翻译（实现见 vision_algo_i18n.cpp）
 */

#include <QString>

namespace vision_algo {

/** 经 QCoreApplication::translate("VisionAlgo", ...) 翻译后返回 */
QString tr(const char *key);

} // namespace vision_algo
