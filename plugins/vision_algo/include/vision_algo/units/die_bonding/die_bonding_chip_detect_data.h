#pragma once

/**
 * @file die_bonding_chip_detect_data.h
 * @brief 固晶缺陷检测单元对外唯一头文件（入参/出参继承 AlgorithmPacket，无 CISDK 头依赖）
 *
 * 调用约定：经 IVisionAlgoService::execute(kUnitDieBondingChipDetect, data) 调度；
 * data.inputs 为 DieBondingChipDetectInput，data.outputs 为 DieBondingChipDetectOutput；
 * 通过 DieBondingChipDetectOp 区分 InitEngine / SyncParam / ScanDlModels / Detect。
 */

#include <vision_algo/core/algorithm_unit_type.h>

#include <common/base/algorithm/algorithm_packet.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace vision_algo {

/** 进程内 CISDK 句柄池默认容量（InitEngine 首次调用后固定，不可再改） */
constexpr std::size_t kDefaultDieBondingPoolSize = 10;

/** RGB 芯片通道，序号与 AOI InspectionChipType 一致 */
enum class ChipChannelType : int
{
    R    = 0, ///< 红色通道
    G    = 1, ///< 绿色通道
    B    = 2, ///< 蓝色通道
    None = 3, ///< 未指定/无效
};

/** 单通道芯片检测阈值（传统算法与 DL 模型共用语义） */
struct ChipThreshold
{
    float chipMatchThreshold   = 0.f; ///< 芯片模板匹配阈值
    int   chipSegmentThreshold = 0;   ///< 芯片分割阈值
    float polarMatchThreshold  = 0.f; ///< 极性检测匹配阈值
};

/** 单颗芯片检测结果（像素坐标与缺陷位标志） */
struct ChipDetectInfo
{
    ChipChannelType chipType       = ChipChannelType::None; ///< 期望芯片类型
    ChipChannelType chipActualType = ChipChannelType::None; ///< 实测芯片类型
    int             polarType      = 2;                     ///< 极性类型（CISDK polar_type）
    AlgoPoint2f     solderCoordinate{};                     ///< 焊盘中心（像素）
    AlgoPoint2f     chipCoordinate{};                       ///< 芯片中心（像素）
    AlgoPoint2f     solderChipDeflection{};                 ///< 焊盘-芯片偏移（像素）
    float           chipAngle            = 0.f;             ///< 芯片角度（度）
    float           chipArea             = 0.f;             ///< 芯片面积（像素平方）
    float           chipWidth            = 0.f;             ///< 芯片宽度（像素）
    float           chipHeight           = 0.f;             ///< 芯片高度（像素）
    float           solderWidth          = 0.f;             ///< 焊盘宽度（像素）
    float           solderHeight         = 0.f;             ///< 焊盘高度（像素）
    int             chipDefect           = 0;               ///< 缺陷位掩码（CISDK chip_defect）
    bool            chipOffsetRow        = false;           ///< 行方向偏移标志
    bool            chipOffsetCol        = false;           ///< 列方向偏移标志
    float           chipOffsetValueRow   = 0.f;             ///< 行方向偏移量
    float           chipOffsetValueCol   = 0.f;             ///< 列方向偏移量
    AlgoRect2f      chipImageRect{};                        ///< 芯片在图像上的 ROI
};

/** 芯片组（一行一列）检测结果 */
struct ChipGroupDetectInfo
{
    int            row = 0;              ///< 组内行索引
    int            col = 0;              ///< 组内列索引
    AlgoRect2f     chipGroupRect{};      ///< 芯片组外接矩形（像素）
    AlgoPoint2f    coordinate{};         ///< 组中心坐标（像素）
    ChipDetectInfo chipRgbInfo[3]{};     ///< RGB 三通道芯片信息，下标 0=R,1=G,2=B
    std::string    imageIndex;           ///< 关联图像索引/FOV 编号
};

/**
 * 固晶检测配方参数（调用方负责 mm 到 px 换算后填入 float 尺寸字段）
 * DL 模型路径字段由 ScanDlModels 或外部扫描填充。
 */
struct DieBondingChipDetectParam
{
    float solderWidth  = 0.0f;  ///< 焊盘宽度（像素）
    float solderHeight = 0.0f;  ///< 焊盘高度（像素）
    float chipWidth    = 0.0f;  ///< 单颗芯片宽度（像素）
    float chipHeight   = 0.0f;  ///< 单颗芯片高度（像素）
    float chipGroupWidth  = 0.0f; ///< 芯片组宽度（像素）
    float chipGroupHeight = 0.0f; ///< 芯片组高度（像素）
    float ledGapX = 0.0f;       ///< 组间水平间距（像素）
    float ledGapY = 0.0f;       ///< 组间垂直间距（像素）
    float chipPitchInGroupMm = 0.0f; ///< 组内芯片间距（毫米，DL 用）
    int   alignType = 0;        ///< 对齐类型（传统 DieBondingChipAlignType）
    float confThr = 0.7f;       ///< 单芯片置信度阈值
    float confGroupThr = 0.5f;  ///< 芯片组置信度阈值
    float chipLimThr = 0.8f;    ///< 芯片缺陷分类阈值
    float solderLimThr = 0.8f;  ///< 焊盘缺陷分类阈值
    float chipGroupMatchThreshold = 0.0f; ///< 芯片组匹配阈值
    float lineDistanceThreshold   = 0.0f; ///< 线距阈值
    ChipThreshold chipThresholdR{};       ///< R 通道阈值
    ChipThreshold chipThresholdG{};       ///< G 通道阈值
    ChipThreshold chipThresholdB{};       ///< B 通道阈值
    std::string chipGroupModelFileName;   ///< 芯片组模型文件路径（传统）
    std::string chipGroupModelName;       ///< 芯片组模板名（传统）
    std::string chipRModelFileName;       ///< R 通道模型文件
    std::string chipRModelName;           ///< R 通道模板名
    std::string chipGModelFileName;       ///< G 通道模型文件
    std::string chipGModelName;           ///< G 通道模板名
    std::string chipBModelFileName;       ///< B 通道模型文件
    std::string chipBModelName;           ///< B 通道模板名
    std::string chipRPolarModelFileName;  ///< R 极性模型文件
    std::string chipRPolarModelName;      ///< R 极性模板名
    std::string chipGPolarModelFileName;  ///< G 极性模型文件
    std::string chipGPolarModelName;      ///< G 极性模板名
    std::string chipBPolarModelFileName;  ///< B 极性模型文件
    std::string chipBPolarModelName;      ///< B 极性模板名
    std::string chipGroupModel;           ///< DL 芯片组模型路径
    std::string chipGroupKey;             ///< DL 芯片组模型键名
    std::string chipDetectModel;          ///< DL 芯片检测模型路径
    std::string chipDetectKey;            ///< DL 芯片检测模型键名
    std::string detectClassifyModel;      ///< DL 缺陷分类模型路径
    std::string detectClassifyKey;        ///< DL 缺陷分类模型键名
    std::string polarDetectModel;         ///< DL 极性检测模型路径
    std::string polarDetectKey;           ///< DL 极性检测模型键名
    int         algoThreadNum = 0;        ///< 算法线程数（0 表示默认）
    /** DL 推理引擎：0=ONNX_CPU，1=ONNX_GPU，2=TensorRT（默认） */
    int         dlInferenceEngine = 2;
};

/** 单张 FOV 固晶检测业务结果汇总 */
struct DieBondingChipDetectResult
{
    std::vector<ChipGroupDetectInfo> chipGroupInfos; ///< 各芯片组明细
    int                              chipGroupNum = 0; ///< 有效芯片组数量
};

/** 固晶检测单元操作类型（写入 DieBondingChipDetectInput::kOp） */
enum class DieBondingChipDetectOp : std::uint8_t
{
    InitEngine   = 0, ///< 创建/复用进程内句柄池（需 kParam、kPoolSize、kDlInferenceEngineOverride）
    SyncParam    = 1, ///< 刷新检测参数，不重建池（需 kParam）
    ScanDlModels = 2, ///< 扫描目录填充 DL 模型路径（需 kParam、kDirectory、kPreferOnnx）
    Detect       = 3, ///< 执行单 FOV 检测（需 kDetectFields；须先 InitEngine 成功）
};

/** 固晶检测入参 Packet */
class DieBondingChipDetectInput : public common::algorithm::AlgorithmPacket
{
public:
    /** Detect 操作的逐 FOV 业务字段 */
    struct DetectFields
    {
        VisionImageContext vision;           ///< 图像与采集上下文
        bool isDeepLearning   = false;       ///< true 走 DL 检测链路
        bool isPolarityDetect = false;       ///< 是否执行极性检测
        bool isDefectClassify = false;       ///< 是否执行缺陷分类
        int  alignType        = 0;           ///< 传统对齐类型
        bool chipMiss          = false;      ///< NG：漏固
        bool chipDirty         = true;       ///< NG：脏污
        bool chipBroken        = true;       ///< NG：破损
        bool chipForeignObject = true;       ///< NG：异物
        bool chipBondingError  = true;       ///< NG：固错
        bool chipPolar         = true;       ///< NG：极性错误
        bool chipSkew          = true;       ///< NG：偏移/歪斜超阈
        bool chipOtherDefect   = true;       ///< NG：其它缺陷位
        int    polar     = 0;                ///< 期望极性类型
        double skewX     = 0.0;              ///< X 方向偏移阈值（微米）
        double skewY     = 0.0;              ///< Y 方向偏移阈值（微米）
        double skewA     = 0.0;              ///< 角度偏移阈值（度）
        double pixelSize = 1.0;              ///< 像素尺寸（微米/像素）
        int    skewType  = 0;                ///< 偏移计算方式（0=焊盘-芯片，1=通道-芯片）
        bool   dlCropLocate = true;          ///< DL 组定位是否裁切
    };

    static constexpr const char kOp[]                        = "op";              ///< 操作类型键
    static constexpr const char kParam[]                     = "param";           ///< 配方参数键
    static constexpr const char kPoolSize[]                  = "poolSize";        ///< 句柄池容量键
    static constexpr const char kDlInferenceEngineOverride[] = "dlOverride";      ///< DL 引擎覆盖键
    static constexpr const char kDirectory[]                 = "directory";     ///< 模型目录键
    static constexpr const char kPreferOnnx[]                  = "preferOnnx";    ///< 优先 ONNX 键
    static constexpr const char kDetectFields[]                = "detectFields";  ///< Detect 字段键

    void setOp(DieBondingChipDetectOp op) { Set(kOp, op); }
    DieBondingChipDetectOp       *op() { return Get<DieBondingChipDetectOp>(kOp); }
    const DieBondingChipDetectOp *op() const { return Get<DieBondingChipDetectOp>(kOp); }

    void setParam(DieBondingChipDetectParam p) { Set(kParam, std::move(p)); }
    DieBondingChipDetectParam       *param() { return Get<DieBondingChipDetectParam>(kParam); }
    const DieBondingChipDetectParam *param() const { return Get<DieBondingChipDetectParam>(kParam); }

    void setPoolSize(std::size_t n) { Set(kPoolSize, n); }
    std::size_t *poolSize() { return Get<std::size_t>(kPoolSize); }

    void setDlInferenceEngineOverride(int v) { Set(kDlInferenceEngineOverride, v); }
    int *dlInferenceEngineOverride() { return Get<int>(kDlInferenceEngineOverride); }

    void setDirectory(std::string dir) { Set(kDirectory, std::move(dir)); }
    std::string *directory() { return Get<std::string>(kDirectory); }

    void setPreferOnnx(bool v) { Set(kPreferOnnx, v); }
    bool *preferOnnx() { return Get<bool>(kPreferOnnx); }

    void setDetectFields(DetectFields f) { Set(kDetectFields, std::move(f)); }
    DetectFields       *detectFields() { return Get<DetectFields>(kDetectFields); }
    const DetectFields *detectFields() const { return Get<DetectFields>(kDetectFields); }
};

/** 固晶检测出参 Packet */
class DieBondingChipDetectOutput : public common::algorithm::AlgorithmPacket
{
public:
    /** Detect 操作的业务结果 */
    struct DetectResult
    {
        VisionResultMeta           meta;    ///< 公共出参元信息（含 ok）
        DieBondingChipDetectResult result;  ///< 芯片组检测明细

        bool ok() const { return meta.ok; }
        void setOk(bool v) { meta.ok = v; }
    };

    static constexpr const char kReady[]        = "ready";        ///< InitEngine/SyncParam：引擎是否就绪
    static constexpr const char kPoolSize[]     = "poolSize";     ///< InitEngine：实际池容量
    static constexpr const char kParam[]        = "param";        ///< ScanDlModels：更新后的参数
    static constexpr const char kDetectResult[] = "detectResult"; ///< Detect：检测结果

    void setReady(bool v) { Set(kReady, v); }
    bool *ready() { return Get<bool>(kReady); }

    void setPoolSize(std::size_t n) { Set(kPoolSize, n); }
    std::size_t *poolSize() { return Get<std::size_t>(kPoolSize); }

    void setParam(DieBondingChipDetectParam p) { Set(kParam, std::move(p)); }
    DieBondingChipDetectParam *param() { return Get<DieBondingChipDetectParam>(kParam); }

    void setDetectResult(DetectResult r) { Set(kDetectResult, std::move(r)); }
    DetectResult       *detectResult() { return Get<DetectResult>(kDetectResult); }
    const DetectResult *detectResult() const { return Get<DetectResult>(kDetectResult); }
};

} // namespace vision_algo
