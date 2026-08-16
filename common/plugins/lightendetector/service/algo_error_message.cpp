/**
 * @file algo_error_message.cpp
 * @brief CISDK AlgoError 模块名与错误码中文描述查询实现。
 */

#include "algo_error_message.h"

#include <common/3rdparty/cisdk/detection/include/ci_detection_modname.h>
#include <common/3rdparty/cisdk/measurement/include/ci_frontend_modname.h>

#include <string>
#include <unordered_map>

namespace common {
namespace algorithm {
namespace {

constexpr uint64_t MakeErrorKey(uint32_t moduleId, uint32_t errorCode)
{
    return (static_cast<uint64_t>(moduleId) << 32) | errorCode;
}

const char* LookupBasicErrorDescription(uint32_t errorCode)
{
    switch (errorCode) {
    case 1:
        return "输入指针为空";
    case 2:
        return "输入数据为空";
    case 3:
        return "输入错误值";
    case 4:
        return "输入数据不匹配";
    case 10:
        return "文件版本错误";
    case 11:
        return "文件后缀错误";
    case 12:
        return "文件大小端不一致";
    case 13:
        return "读取文件失败";
    case 14:
        return "写入文件失败";
    case 15:
        return "文件大小不匹配";
    case 16:
        return "缓冲区大小不足";
    case 17:
        return "校验模块码不匹配";
    case 20:
        return "数据为空";
    case 21:
        return "结构体信息头错误";
    case 22:
        return "无效方法";
    default:
        return nullptr;
    }
}

const std::unordered_map<uint32_t, const char*>& ModuleDescriptionMap()
{
    static const std::unordered_map<uint32_t, const char*> kMap = {
        {ALGOMOD_ALGOLOG, "日志模块"},
        {ALGOMOD_ALGOMAT, "AlgoMat"},
        {ALGOMOD_UTILS, "utils模块"},
        {ALGOMOD_ALGOSHOT, "shot模块"},
        {ALGOMOD_ROI_DETECTOR, "区域定位"},
        {ALGOMOD_QUALITY_ESTIMATOR, "质量评价"},
        {ALGOMOD_LED_POINT_LOCATOR, "点定位"},
        {ALGOMOD_LED_COUTOUR_SEARCHER, "灯点框模块"},
        {ALGOMOD_LED_LUM_EXTRACTOR, "光通量提取"},
        {ALGOMOD_ALGOLEDMETER, "ledmeter模块"},
        {ALGOMOD_VIGNETTING_CALIB, "渐晕标定模块"},
        {ALGOMOD_BLUR_LED_LUM_EXTRACTOR, "模糊光通量提取"},
        {ALGOMOD_GEOMETRY_TRANSFORMER, "几何变换"},
        {ALGOMOD_CAMERA_CONTROLLER, "相机控制"},
        {ALGOMOD_UNIFORM_FIELD_CALIB, "均匀场标定"},
        {ALGOMOD_POINT_FINDER, "灯点搜索模块"},
        {ALGOMOD_CHESSBOARD_DETECTOR, "棋盘格检测"},
        {ALGOMOD_COLORIMETER_CALIB, "色度计标定"},
        {ALGOMOD_EDGE_CALIB, "边缘校正"},
        {ALGOMOD_CURVE_CALIB, "曲面特征标定修正"},
        {ALGOMOD_CAMERA_CHROMA_CALIB, "相机色度标定"},
        {ALGOMOD_CAMERA_LUMINANCE_CALIBRATION, "相机亮度标定"},
        {ALGOMOD_LUM_ACCURACY_OPTIMIZER, "光通量PSF"},
        {ALGOMOD_POINT_IRREGULAR_MAPPER, "异形屏灯点排列模块"},
        {ALGOMOD_POING_MAPPER, "点定位匹配"},
        {ALGOMOD_SHOT_UTILS, "shot工具"},
        {ALGOMOD_POINT_MAPPER, "灯点排列模块"},
        {ALGOMOD_MAPPER_UTILS, "mapper结果工具库"},
        {ALGOMOD_LED_IRREGULAR_COUTOUR_SEARCHER, "多边形框搜索模块"},
        {ALGOMOD_MOBILE_CAMERA_CONTROLLER, "移动端相机调节模块"},
        {ALGOMOD_DISTOTTION_CORRECTION, "相机畸变标定"},
        {ALGOMOD_CMF_CALIB, "多光谱标定"},
        {ALGOMOD_THERMAL_IMAGER, "温度数据模块"},
        {ALGOMOD_SURFACE_TEMPERATURE_CALIB, "环境温度补偿"},
        {ALGOMOD_LUMCHROMA_DETECTOR, "亮色度检测"},
        {ALGOMOD_STABILITY_MONITOR, "稳定性监控模块"},
        {ALGOMOD_LED_METER_INSPECTOR, "灯点点亮缺陷检测"},
    };
    return kMap;
}

const std::unordered_map<uint64_t, const char*>& ModuleErrorDescriptionMap()
{
    static const std::unordered_map<uint64_t, const char*> kMap = {
        // ALGOMOD_ALGOSHOT
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 100), "相机类型错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 101), "shot类型和数据不匹配"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 102), "数据不在相同拍摄序列"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 103), "点亮灯点宽高错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 104), "Shot pattern信息错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 105), "Shot device信息错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 106), "Shot panel信息错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 107), "Shot capture信息错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 108), "Shot partition信息错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 109), "Shot mat类型错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 110), "Shot CIEtype类型错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 111), "Shot color信息错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 112), "Shot bayer信息错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 113), "Shot区域信息错误"},
        {MakeErrorKey(ALGOMOD_ALGOSHOT, 114), "Shot业务操作记录数据错误"},

        // ALGOMOD_ROI_DETECTOR
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 100), "未找到ROI"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 101), "角点个数不为4"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 102), "需先计算变换矩阵"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 103), "角点超出图像范围"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 104), "未找到4个角点"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 105), "角点重复"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 106), "拼接屏坐标顺序错误"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 107), "拼接屏数量有误"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 108), "模组数量有误"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 109), "待分区颜色数量有误"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 110), "分区模式有误"},
        {MakeErrorKey(ALGOMOD_ROI_DETECTOR, 111), "thermal_imager数据类型错误"},

        // ALGOMOD_POINT_FINDER
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 100), "并行线程数设置错误"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 101), "匹配方法设置错误"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 102), "下采样率设置错误"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 103), "亚像素定位标志设置错误"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 104), "点定位方法设置错误"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 105), "模板行列数设置错误"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 106), "模板图像为空"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 107), "输入图像为空"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 108), "ROI区域超出图像范围"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 109), "写入图像失败"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 110), "Mask与图像尺寸不匹配"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 111), "Mask全为0"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 112), "模板参数范围错误"},
        {MakeErrorKey(ALGOMOD_POINT_FINDER, 113), "后处理参数范围错误"},

        // ALGOMOD_POINT_MAPPER
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 100), "CPU并行线程数错误"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 101), "输入点集类型不匹配"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 102), "初始化种子点失败"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 103), "寻找初始行失败"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 104), "输入灯点行列数错误"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 105), "行列数与输入不匹配"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 106), "灯点大面积缺失"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 107), "无法计算死灯点坐标"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 108), "输入图像为空"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 109), "写入图像失败"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 110), "map信息为空"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 111), "四角点附近灯点为空"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 112), "有效四角点数量不足"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 113), "排列方法不匹配"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 114), "死灯预测越界"},
        {MakeErrorKey(ALGOMOD_POINT_MAPPER, 115), "输入灯点数量异常"},

        // ALGOMOD_LED_COUTOUR_SEARCHER
        {MakeErrorKey(ALGOMOD_LED_COUTOUR_SEARCHER, 100), "输入shot信息为空"},
        {MakeErrorKey(ALGOMOD_LED_COUTOUR_SEARCHER, 101), "输入点定位信息为空"},
        {MakeErrorKey(ALGOMOD_LED_COUTOUR_SEARCHER, 102), "输入灯点排布信息为空"},
        {MakeErrorKey(ALGOMOD_LED_COUTOUR_SEARCHER, 103), "CPU并行线程数错误"},
        {MakeErrorKey(ALGOMOD_LED_COUTOUR_SEARCHER, 104), "搜索框方法错误"},
        {MakeErrorKey(ALGOMOD_LED_COUTOUR_SEARCHER, 105), "灯点框缩放参数错误"},
        {MakeErrorKey(ALGOMOD_LED_COUTOUR_SEARCHER, 106), "contour信息为空"},
        {MakeErrorKey(ALGOMOD_LED_COUTOUR_SEARCHER, 107), "contour结果超出图像边界"},

        // ALGOMOD_LED_LUM_EXTRACTOR
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 100), "contour内参类型不匹配"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 101), "光通量提取方法错误"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 102), "输入shot信息为空"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 103), "shot通道数错误"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 104), "输入contour信息为空"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 105), "输入roi信息为空"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 106), "dc参数错误"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 107), "dc范围超出shot区域"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 108), "光通量计算结果为空"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 109), "ir_contour内参类型不匹配"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 110), "ir_contour信息为空"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 111), "透视变换后灯点框超界"},
        {MakeErrorKey(ALGOMOD_LED_LUM_EXTRACTOR, 112), "contour超出图像边界"},

        // ALGOMOD_ALGOLEDMETER
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 100), "点亮灯点个数不一致"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 101), "重复添加数据"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 102), "输入元信息不全"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 103), "输入为多通道"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 104), "输入系列不匹配"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 105), "Align参数无效"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 106), "子区域宽高错误"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 107), "亮暗点检测阈值错误"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 108), "输入meter数量异常"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 109), "meter拼接行列数异常"},
        {MakeErrorKey(ALGOMOD_ALGOLEDMETER, 110), "meter业务操作记录数据错误"},
    };
    return kMap;
}

} // namespace

const char* GetAlgoModuleDescription(uint32_t moduleId)
{
    const auto& map = ModuleDescriptionMap();
    const auto it = map.find(moduleId);
    if (it != map.end()) {
        return it->second;
    }

    thread_local std::string fallback;
    fallback = "模块" + std::to_string(moduleId);
    return fallback.c_str();
}

const char* GetAlgoErrorDescription(uint32_t moduleId, uint32_t errorCode)
{
    if (const char* basic = LookupBasicErrorDescription(errorCode); basic != nullptr) {
        return basic;
    }

    const auto& map = ModuleErrorDescriptionMap();
    const auto key = MakeErrorKey(moduleId, errorCode);
    const auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }

    thread_local std::string fallback;
    fallback = "错误码" + std::to_string(errorCode);
    return fallback.c_str();
}

} // namespace algorithm
} // namespace common
