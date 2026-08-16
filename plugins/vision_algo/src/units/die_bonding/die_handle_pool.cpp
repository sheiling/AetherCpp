#include "die_handle_pool.h"

/**
 * @file die_handle_pool.cpp
 * @brief CISDK 固晶检测句柄池：预创建、线程绑定、动态扩容与参数/模型同步
 */

#include "algo_ci_data_convert.h"
#include "algo_ci_error_check.h"
#include "dl_model_scanner.h"

#include <Logger.h>

#include <QString>

#include <ci_basic_error.h>
#include <ci_utils_algomat.h>
#include <dl_die_bonding_detector.h>
#include <model_processor.h>

#include <iostream>
#include <sstream>
#include <thread>

namespace vision_algo {

/** 构造：可选扫描 dlmodel → 预创建 poolSize 个句柄 → setParamModel */
HandlePool::HandlePool(size_t poolSize, bool isDeepLearning, const DieBondingChipDetectParam &param,
                       const std::string &dlModelDir)
    : m_isDeepLearning(isDeepLearning)
    , m_param(param)
    , m_dlModelDir(dlModelDir)
    , m_initialPoolSize(poolSize)
{
    // 预创建 initialPoolSize 个句柄并加载模型
    LOG_INFO("[句柄池] HandlePool ctor begin poolSize={} dlDir={}", poolSize, m_dlModelDir);
    if (m_isDeepLearning && m_param.chipGroupModel.empty()) {
        QString scanError;
        if (!vision_algo::scanDlModelDirectory(QString::fromStdString(m_dlModelDir), m_param,
                                             nullptr, &scanError)) {
            LOG_ERROR("[句柄池] 扫描 dlmodel 目录失败：{}", scanError.toStdString());
        }
    }
    LOG_INFO("[句柄池] HandlePool models group={} detect={}", m_param.chipGroupModel,
             m_param.chipDetectModel);

    m_handles.resize(poolSize);
    for (size_t i = 0; i < poolSize; ++i) {
        m_handles[i].m_handle   = nullptr;
        m_handles[i].m_threadID = 0;
        m_handles[i].m_valid    = false;

        LOG_INFO("[句柄池] HandlePool createHandle {}/{}", i + 1, poolSize);
        if (const auto err = createHandle(m_handles[i].m_handle); err) {
            LOG_ERROR("[句柄池] 初始化 createHandle 失败，序号={}，原因={}", i + 1, err->Message());
        } else {
            m_handles[i].m_valid = true;
            LOG_INFO("[句柄池] createHandle ok index={}", i + 1);
        }
    }

    for (auto &handle : m_handles) {
        if (!handle.m_valid || !handle.m_handle)
            continue;
        if (const auto err = setParamModel(handle.m_handle); err) {
            LOG_ERROR("[句柄池] 初始化 setParamModel 失败：{}", err->Message());
            handle.m_valid = false;
        }
    }
    LOG_INFO("[句柄池] HandlePool ctor end");
}

/** 析构：销毁 vector 内全部 CISDK 句柄 */
HandlePool::~HandlePool()
{
    for (auto &handle : m_handles) {
        if (handle.m_handle) {
            destroyHandle(handle.m_handle);
            handle.m_valid = false;
        }
    }
}

AlgoHandle HandlePool::GetHandle()
{
    // CISDK 句柄与线程绑定：将 thread::id 转为 int 作槽位键（与历史实现一致）
    std::thread::id this_id = std::this_thread::get_id();
    int             id      = *reinterpret_cast<unsigned int *>(&this_id);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        // 1) 已绑定当前线程 → 直接复用
        for (auto &handle : m_handles) {
            if (handle.m_valid && (handle.m_threadID == id)) {
                return handle.m_handle;
            }
        }
        // 2) 空闲槽（m_threadID==0）→ 绑定到当前线程
        for (auto &handle : m_handles) {
            if (handle.m_valid && (handle.m_threadID == 0)) {
                handle.m_threadID = id;
                return handle.m_handle;
            }
        }
    }

    // 3) 初始池已满：动态扩容（最多重试 5 次 createHandle）
    AlgoHandle algoHandle = nullptr;
    for (int i = 0; i < 5; ++i) {
        if (const auto err = createHandle(algoHandle); err) {
            LOG_ERROR("[句柄池] 动态 createHandle 失败：{}", err->Message());
            destroyHandle(algoHandle);
            algoHandle = nullptr;
            continue;
        }
        break;
    }
    if (algoHandle == nullptr) {
        LOG_ERROR("[句柄池] 动态扩容失败（initialPoolSize={} current={}）", m_initialPoolSize,
                  m_handles.size());
        return nullptr;
    }
    if (const auto err = setParamModel(algoHandle); err) {
        LOG_ERROR("[句柄池] 动态 setParamModel 失败：{}", err->Message());
        destroyHandle(algoHandle);
        return nullptr;
    }

    LOG_WARN("[句柄池] 动态扩容句柄 initialPoolSize={} → {}", m_initialPoolSize,
             m_handles.size() + 1);
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_handles.push_back({algoHandle, id, true});
        return m_handles.back().m_handle;
    }
}

/** 更新 m_param 并刷新池内全部有效句柄的参数（不重载模型） */
common::errors::ErrorPtr HandlePool::SetParam(const DieBondingChipDetectParam &param)
{
    m_param = param;
    for (auto &handle : m_handles) {
        if (!handle.m_valid || !handle.m_handle)
            continue;
        if (const auto err = setParamModel(handle.m_handle); err)
            return err;
    }
    return nullptr;
}

/** 创建 CISDK 句柄并 loadModels（DL 或传统） */
common::errors::ErrorPtr HandlePool::createHandle(AlgoHandle &handle)
{
    AlgoHandle detector_handle = nullptr;
    AlgoError  err             = ALGO_PASS;
    if (m_isDeepLearning) {
#ifdef ENABLE_DL_BONDING_DETECT
        LOG_INFO("[句柄池] DLDieBondingCreateHandle");
        err = DLDieBondingCreateHandle(&detector_handle);
        CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingCreateHandle", err));
#else
        return algoErrorFromKey(common::errors::algo_module::kSdkError,
                                "vision_algo.error.dl_bonding_disabled");
#endif
    } else {
        err = DieBondingCreateHandle(&detector_handle);
        CHECK_RTN_INFO_LOG(checkAlgoError("DieBondingCreateHandle", err));
    }

    if (detector_handle == nullptr) {
        return algoErrorFromKey(common::errors::algo_module::kInvalidHandle,
                                "vision_algo.error.invalid_handle_null");
    }
    handle = detector_handle;

    if (m_isDeepLearning) {
        LOG_INFO("[句柄池] loadModelsDL begin");
        if (const auto loadErr = loadModelsDL(handle); loadErr) {
            LOG_INFO("[句柄池] loadModelsDL end failed");
            return loadErr;
        }
        LOG_INFO("[句柄池] loadModelsDL end ok");
        return nullptr;
    }
    if (const auto loadErr = loadModels(handle); loadErr)
        return loadErr;
    return setParamModel(handle);
}

void HandlePool::destroyHandle(AlgoHandle handle)
{
    if (!handle)
        return;
    if (m_isDeepLearning) {
#ifdef ENABLE_DL_BONDING_DETECT
        DLDieBondingDestroyHandle(handle);
#endif
    } else {
        DieBondingDestroyHandle(handle);
    }
}

common::errors::ErrorPtr HandlePool::setParamModel(AlgoHandle handle)
{
    if (m_isDeepLearning)
        return setDieParamDL(handle);
    return setDieParam(handle);
}

common::errors::ErrorPtr HandlePool::setDieParamDL(AlgoHandle handle)
{
#ifdef ENABLE_DL_BONDING_DETECT
    // AlgoSize 为 int 像素；配方 float 尺寸传入 CISDK 前显式取整
    DLDieBondingParameter para{{static_cast<int>(m_param.solderWidth),
                                static_cast<int>(m_param.solderHeight)},
                               {static_cast<int>(m_param.chipWidth),
                                static_cast<int>(m_param.chipHeight)},
                               {static_cast<int>(m_param.chipGroupWidth),
                                static_cast<int>(m_param.chipGroupHeight)},
                               {static_cast<int>(m_param.ledGapX),
                                static_cast<int>(m_param.ledGapY)},
                               DieBondingChipAlignType(m_param.alignType)};
    LOG_INFO(
        "[句柄池] setDieParamDL → DLDieBondingSetParameter: "
        "solder={}x{} chip={}x{} group={}x{} led_gap={}x{} align={}",
        para.solder_size.width, para.solder_size.height, para.chip_size.width, para.chip_size.height,
        para.chip_group_size.width, para.chip_group_size.height, para.led_gap.width,
        para.led_gap.height, static_cast<int>(para.chip_align_type));

    AlgoError err = DLDieBondingSetParameter(handle, &para);
    CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingSetParameter", err));

    err = DLDieBondingSetGroupConfThresh(handle, m_param.confGroupThr);
    CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingSetGroupConfThresh", err));

    err = DLDieBondingSetChipConfThresh(handle, m_param.confThr);
    CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingSetChipConfThresh", err));

    err = DLDieBondingSetDefectThresh(handle, m_param.chipLimThr, m_param.solderLimThr);
    CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingSetDefectThresh", err));

    LOG_INFO(
        "[句柄池] setDieParamDL thresholds: conf_group={:.3f} conf_chip={:.3f} "
        "chip_lim={:.3f} solder_lim={:.3f}",
        m_param.confGroupThr, m_param.confThr, m_param.chipLimThr, m_param.solderLimThr);
#else
    (void)handle;
    return algoErrorFromKey(common::errors::algo_module::kSdkError,
                            "vision_algo.error.dl_bonding_disabled");
#endif
    return nullptr;
}

common::errors::ErrorPtr HandlePool::setDieParam(AlgoHandle handle)
{
    // AlgoSize 为 int 像素；配方 float 尺寸传入 CISDK 前显式取整
    DieBondingParameter in_parameter{{static_cast<int>(m_param.solderWidth),
                                    static_cast<int>(m_param.solderHeight)},
                                   {static_cast<int>(m_param.chipWidth),
                                    static_cast<int>(m_param.chipHeight)},
                                   {static_cast<int>(m_param.ledGapX),
                                    static_cast<int>(m_param.ledGapY)}};
    const DieBondingChipAlignType align_type = DieBondingChipAlignType(m_param.alignType);

    AlgoError err = DieBondingSetParameter(handle, &in_parameter, align_type);
    CHECK_RTN_INFO_LOG(checkAlgoError("DieBondingSetParameter", err));

    DieBondingGroupThreshold group_threshold{m_param.chipGroupMatchThreshold,
                                             m_param.lineDistanceThreshold};
    err = DieBondingSetGroupThresh(handle, &group_threshold);
    CHECK_RTN_INFO_LOG(checkAlgoError("DieBondingSetGroupThresh", err));

    DieBondingChipThreshold thrR = cisdk_convert::toCisdkThreshold(m_param.chipThresholdR);
    DieBondingChipThreshold thrG = cisdk_convert::toCisdkThreshold(m_param.chipThresholdG);
    DieBondingChipThreshold thrB = cisdk_convert::toCisdkThreshold(m_param.chipThresholdB);

    err = DieBondingSetChipThresh(handle, &thrR, CHIP_R);
    CHECK_RTN_INFO_LOG(checkAlgoError("DieBondingSetChipThresh(CHIP_R)", err));

    err = DieBondingSetChipThresh(handle, &thrG, CHIP_G);
    CHECK_RTN_INFO_LOG(checkAlgoError("DieBondingSetChipThresh(CHIP_G)", err));

    err = DieBondingSetChipThresh(handle, &thrB, CHIP_B);
    CHECK_RTN_INFO_LOG(checkAlgoError("DieBondingSetChipThresh(CHIP_B)", err));

    return nullptr;
}

common::errors::ErrorPtr HandlePool::loadModelsDL(AlgoHandle handle)
{
    if (!handle) {
        return algoErrorFromKey(common::errors::algo_module::kInvalidHandle,
                                "vision_algo.error.invalid_handle_null");
    }
#ifdef ENABLE_DL_BONDING_DETECT
    int i = 0;
    LOG_INFO("[句柄池] 加载 DL 模型，步骤 {}", i++);
    m_szmutex[0].lock();
    LOG_INFO("[句柄池] LoadModel step1 CHIP_GROUP_LOCATE path={} engine={}",
             m_param.chipGroupModel, m_param.dlInferenceEngine);
    AlgoError err = DLDieBondingLoadModel(handle, m_param.chipGroupModel.c_str(),
                                          m_param.chipGroupKey.c_str(), CHIP_GROUP_LOCATE_MODEL, 4,
                                          m_param.dlInferenceEngine);
    m_szmutex[0].unlock();
    CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingLoadModel(CHIP_GROUP_LOCATE)", err));

    LOG_INFO("[句柄池] 加载 DL 模型，步骤 {}", i++);
    m_szmutex[2].lock();
    err = DLDieBondingLoadModel(handle, m_param.chipDetectModel.c_str(), m_param.chipDetectKey.c_str(),
                                CHIP_DETECT_MODEL, 64, m_param.dlInferenceEngine);
    m_szmutex[2].unlock();
    CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingLoadModel(CHIP_DETECT)", err));

    LOG_INFO("[句柄池] 加载 DL 模型，步骤 {}", i++);
    if (!m_param.detectClassifyModel.empty()) {
        m_szmutex[3].lock();
        err = DLDieBondingLoadModel(handle, m_param.detectClassifyModel.c_str(),
                                    m_param.detectClassifyKey.c_str(), CHIP_DEFECT_CLASSIFY_MODEL, 64,
                                    m_param.dlInferenceEngine);
        m_szmutex[3].unlock();
        CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingLoadModel(CHIP_DEFECT_CLASSIFY)", err));
    }

    LOG_INFO("[句柄池] 加载 DL 模型，步骤 {}", i++);
    if (!m_param.polarDetectModel.empty()) {
        m_szmutex[1].lock();
        err = DLDieBondingLoadModel(handle, m_param.polarDetectModel.c_str(),
                                    m_param.polarDetectKey.c_str(), CHIP_POLARITY_CLASSIFY_MODEL, 64,
                                    m_param.dlInferenceEngine);
        m_szmutex[1].unlock();
        CHECK_RTN_INFO_LOG(checkAlgoError("DLDieBondingLoadModel(CHIP_POLARITY)", err));
    }

    LOG_INFO("[句柄池] DLDieBondingLoadModel 全部完成");
#else
    (void)handle;
    return algoErrorFromKey(common::errors::algo_module::kSdkError,
                            "vision_algo.error.dl_bonding_disabled");
#endif
    return nullptr;
}

common::errors::ErrorPtr HandlePool::loadModels(AlgoHandle handle)
{
    if (!handle) {
        return algoErrorFromKey(common::errors::algo_module::kInvalidHandle,
                                "vision_algo.error.invalid_handle_null");
    }

    AlgoHandle group_model_handle        = ModelCreateHandle();
    AlgoHandle r_chip_model_handle       = ModelCreateHandle();
    AlgoHandle g_chip_model_handle       = ModelCreateHandle();
    AlgoHandle b_chip_model_handle       = ModelCreateHandle();
    AlgoHandle g_chip_palar_model_handle = ModelCreateHandle();
    AlgoHandle b_chip_palar_model_handle = ModelCreateHandle();

    auto cleanup = [&]() {
        if (group_model_handle)
            ModelDestroyHandle(group_model_handle);
        if (r_chip_model_handle)
            ModelDestroyHandle(r_chip_model_handle);
        if (g_chip_model_handle)
            ModelDestroyHandle(g_chip_model_handle);
        if (b_chip_model_handle)
            ModelDestroyHandle(b_chip_model_handle);
        if (g_chip_palar_model_handle)
            ModelDestroyHandle(g_chip_palar_model_handle);
        if (b_chip_palar_model_handle)
            ModelDestroyHandle(b_chip_palar_model_handle);
    };

    AlgoError err = ModelLoad(group_model_handle, m_param.chipGroupModelFileName.c_str());
    if (err == ALGO_PASS)
        err = ModelLoad(r_chip_model_handle, m_param.chipRModelFileName.c_str());
    if (err == ALGO_PASS)
        err = ModelLoad(g_chip_model_handle, m_param.chipGModelFileName.c_str());
    if (err == ALGO_PASS)
        err = ModelLoad(b_chip_model_handle, m_param.chipBModelFileName.c_str());
    if (err == ALGO_PASS)
        err = ModelLoad(g_chip_palar_model_handle, m_param.chipGPolarModelFileName.c_str());
    if (err == ALGO_PASS)
        err = ModelLoad(b_chip_palar_model_handle, m_param.chipBPolarModelFileName.c_str());
    if (err == ALGO_PASS) {
        err = DieBondingSetGroupTemplate(handle, group_model_handle,
                                         m_param.chipGroupModelName.c_str());
    }
    if (err == ALGO_PASS) {
        err = DieBondingSetChipTemplate(handle, r_chip_model_handle, m_param.chipRModelName.c_str(),
                                        CHIP_R, CHIP_TEMPLATE);
    }
    if (err == ALGO_PASS) {
        err = DieBondingSetChipTemplate(handle, g_chip_model_handle, m_param.chipGModelName.c_str(),
                                        CHIP_G, CHIP_TEMPLATE);
    }
    if (err == ALGO_PASS) {
        err = DieBondingSetChipTemplate(handle, g_chip_palar_model_handle,
                                        m_param.chipGPolarModelName.c_str(), CHIP_G, POLAR_TEMPLATE);
    }
    if (err == ALGO_PASS) {
        err = DieBondingSetChipTemplate(handle, b_chip_model_handle, m_param.chipBModelName.c_str(),
                                        CHIP_B, CHIP_TEMPLATE);
    }
    if (err == ALGO_PASS) {
        err = DieBondingSetChipTemplate(handle, b_chip_palar_model_handle,
                                        m_param.chipBPolarModelName.c_str(), CHIP_B, POLAR_TEMPLATE);
    }

    cleanup();

    if (err != ALGO_PASS) {
        return checkAlgoError("DieBondingLoadModel", err);
    }

    LOG_INFO("[句柄池] DieBondingLoadModel 完成");
    return nullptr;
}

} // namespace vision_algo
