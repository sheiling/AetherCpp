#pragma once

/**
 * @file dl_model_scanner.h
 * @brief 固晶 DL 模型目录扫描（插件内部，仅 DL 固晶使用）
 *
 * 约定文件名前缀：chip_group_encry*、chip_detect_encry*、polar_detect_encry*、defect_cls_encry*
 * 配套 .txt 为加密 key。
 */

#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include <QString>
#include <QStringList>

namespace vision_algo {

/**
 * @brief 扫描目录并填充 param 中 DL 模型路径与 key
 * @param directory  dlmodel 根目录；为空时使用 exe 旁 dlmodel/ 默认路径
 * @param param      输入输出参数结构
 * @param outFound   可选：命中的文件名列表
 * @param error      失败原因
 * @param preferOnnx 同前缀下 onnx 优先于 trt
 */
bool scanDlModelDirectory(const QString &directory, DieBondingChipDetectParam &param,
                          QStringList *outFound = nullptr, QString *error = nullptr,
                          bool preferOnnx = true);

} // namespace vision_algo
