#include "die_bonding_engine_manager.h"

#include "die_bonding_detect_engine.h"
#include "vision_algo_i18n.h"

#include <Logger.h>

namespace vision_algo {
namespace die_bonding_internal {
namespace {

DieBondingDetectEnginePtr g_engine;
std::size_t               g_poolSize = 0;
DieBondingChipDetectParam g_param{};

DieBondingDetectEnginePtr tryCreateEngine(const DieBondingChipDetectParam &param,
                                          std::string                     *errorOut)
{
#ifdef VISION_ALGO_HAS_DIE_BONDING
    try {
        const bool useDeepLearning =
#ifdef VISION_ALGO_HAS_DL_DIE_BONDING
            param.dlInferenceEngine > 0;
#else
            false;
#endif
        auto engine =
            std::make_shared<DieBondingDetectEngine>(param, g_poolSize, useDeepLearning);
        if (!engine->isReady()) {
            if (errorOut)
                *errorOut = tr("vision_algo.error.engine_not_initialized").toStdString();
            return {};
        }
        return engine;
    } catch (const std::exception &ex) {
        if (errorOut) {
            *errorOut = tr("vision_algo.error.engine_create_failed")
                            .arg(QString::fromUtf8(ex.what()))
                            .toStdString();
        }
        return {};
    }
#else
    (void)param;
    if (errorOut)
        *errorOut = tr("vision_algo.error.die_bonding_disabled").toStdString();
    return {};
#endif
}

} // namespace

bool acquireEngine(const DieBondingChipDetectParam &param, std::size_t poolSize,
                   int dlInferenceEngineOverride, std::string *errorOut, std::size_t *outPoolSize)
{
#ifdef VISION_ALGO_HAS_DIE_BONDING
    DieBondingChipDetectParam engineParam = param;
    if (dlInferenceEngineOverride >= 0)
        engineParam.dlInferenceEngine = dlInferenceEngineOverride;

    if (!g_engine) {
        g_poolSize = poolSize > 0 ? poolSize : kDefaultDieBondingPoolSize;
        LOG_INFO("[固晶引擎] 首次创建句柄池 poolSize={}", g_poolSize);
        g_engine = tryCreateEngine(engineParam, errorOut);
        if (g_engine)
            g_param = engineParam;
        if (outPoolSize)
            *outPoolSize = g_poolSize;
        return g_engine && g_engine->isReady();
    }

    if (poolSize > 0 && poolSize != g_poolSize) {
        LOG_WARN("[固晶引擎] 忽略 poolSize 变更请求 {}（已固定为 {}）", poolSize, g_poolSize);
    }

    if (engineParam.dlInferenceEngine != g_param.dlInferenceEngine
        || engineParam.chipGroupModel != g_param.chipGroupModel
        || engineParam.chipDetectModel != g_param.chipDetectModel
        || engineParam.detectClassifyModel != g_param.detectClassifyModel
        || engineParam.polarDetectModel != g_param.polarDetectModel) {
        LOG_INFO("[固晶引擎] 模型路径变更，重建句柄池（poolSize 保持 {}）", g_poolSize);
        g_engine.reset();
        g_engine = tryCreateEngine(engineParam, errorOut);
        if (!g_engine) {
            if (outPoolSize)
                *outPoolSize = g_poolSize;
            return false;
        }
        g_param = engineParam;
        if (outPoolSize)
            *outPoolSize = g_poolSize;
        return true;
    }

    if (!g_engine->syncParam(engineParam)) {
        if (errorOut)
            *errorOut = tr("vision_algo.error.engine_sync_param_failed").toStdString();
        LOG_ERROR("[固晶引擎] syncParam 失败");
        if (outPoolSize)
            *outPoolSize = g_poolSize;
        return g_engine->isReady();
    }
    g_param = engineParam;
    if (outPoolSize)
        *outPoolSize = g_poolSize;
    return g_engine->isReady();
#else
    (void)param;
    (void)poolSize;
    (void)dlInferenceEngineOverride;
    if (errorOut)
        *errorOut = tr("vision_algo.error.die_bonding_disabled").toStdString();
    if (outPoolSize)
        *outPoolSize = 0;
    return false;
#endif
}

bool syncEngineParam(const DieBondingChipDetectParam &param, std::string *errorOut)
{
#ifdef VISION_ALGO_HAS_DIE_BONDING
    if (!g_engine) {
        if (errorOut)
            *errorOut = tr("vision_algo.error.engine_not_initialized").toStdString();
        return false;
    }
    if (!g_engine->syncParam(param)) {
        if (errorOut)
            *errorOut = tr("vision_algo.error.engine_sync_param_failed").toStdString();
        return false;
    }
    g_param = param;
    return true;
#else
    (void)param;
    if (errorOut)
        *errorOut = tr("vision_algo.error.die_bonding_disabled").toStdString();
    return false;
#endif
}

DieBondingDetectEngine *activeEngine()
{
#ifdef VISION_ALGO_HAS_DIE_BONDING
    return g_engine.get();
#else
    return nullptr;
#endif
}

bool engineReady()
{
#ifdef VISION_ALGO_HAS_DIE_BONDING
    return g_engine && g_engine->isReady();
#else
    return false;
#endif
}

std::size_t enginePoolSize()
{
#ifdef VISION_ALGO_HAS_DIE_BONDING
    return g_engine ? g_engine->poolSize() : 0;
#else
    return 0;
#endif
}

} // namespace die_bonding_internal
} // namespace vision_algo
