#include "vision_algo_service_impl.h"

#include "algo_ci_error_check.h"
#include "vision_algo_i18n.h"

#include <vision_algo/core/service_access.h>
#include <vision_algo/units/marker/mark_point_locate_data.h>
#include <vision_algo/units/model/model_processor_data.h>

#include <common/base/algorithm/algorithm_factory.h>

#include <future>

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>

#include <Logger.h>

namespace vision_algo {
namespace {

/** 经 execute(SdkReady) 查询 model_processor SDK 是否加载成功 */
bool queryModelSdkReady(IVisionAlgoService *svc)
{
    if (!svc || !svc->isReady())
        return false;
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::SdkReady);
    common::algorithm::AlgorithmExecuteData data;
    data.inputs = inPkt;
    if (svc->execute(kUnitModelProcessor, data))
        return false;
    if (auto *ready = data.outputs.Get<bool>(vision_algo::ModelProcessorOutput::kSdkReady))
        return *ready;
    return false;
}

/** 经 execute(SdkLoadDetail) 获取 model_processor SDK 加载详情 */
QString queryModelSdkDetail(IVisionAlgoService *svc)
{
    if (!svc || !svc->isReady())
        return tr("vision_algo.error.service_not_ready");
    vision_algo::ModelProcessorInput inPkt;
    inPkt.setOp(vision_algo::ModelProcessorOp::SdkLoadDetail);
    common::algorithm::AlgorithmExecuteData data;
    data.inputs = inPkt;
    if (const auto err = svc->execute(kUnitModelProcessor, data))
        return QString::fromStdString(err->Message());
    if (auto *detail = data.outputs.Get<std::string>(vision_algo::ModelProcessorOutput::kSdkDetail))
        return QString::fromStdString(*detail);
    return {};
}

/** 经 execute(SdkReady) 查询 marker_detector SDK 是否加载成功 */
bool queryMarkerSdkReady(IVisionAlgoService *svc)
{
    if (!svc || !svc->isReady())
        return false;
    vision_algo::MarkPointLocateInput inPkt;
    inPkt.setOp(vision_algo::MarkPointLocateOp::SdkReady);
    common::algorithm::AlgorithmExecuteData data;
    data.inputs = inPkt;
    if (svc->execute(kUnitMarkPointLocate, data))
        return false;
    if (auto *ready = data.outputs.Get<bool>(vision_algo::MarkPointLocateOutput::kSdkReady))
        return *ready;
    return false;
}

/** 经 execute(SdkLoadDetail) 获取 marker_detector SDK 加载详情 */
QString queryMarkerSdkDetail(IVisionAlgoService *svc)
{
    if (!svc || !svc->isReady())
        return tr("vision_algo.error.service_not_ready");
    vision_algo::MarkPointLocateInput inPkt;
    inPkt.setOp(vision_algo::MarkPointLocateOp::SdkLoadDetail);
    common::algorithm::AlgorithmExecuteData data;
    data.inputs = inPkt;
    if (const auto err = svc->execute(kUnitMarkPointLocate, data))
        return QString::fromStdString(err->Message());
    if (auto *detail = data.outputs.Get<std::string>(vision_algo::MarkPointLocateOutput::kSdkDetail))
        return QString::fromStdString(*detail);
    return {};
}

/** 拼接多行错误说明，跳过空尾行 */
QString appendLines(const QString &head, const QString &tail)
{
    if (tail.isEmpty())
        return head;
    if (head.isEmpty())
        return tail;
    return head + QLatin1Char('\n') + tail;
}

} // namespace

VisionAlgoServiceImpl::VisionAlgoServiceImpl(QObject *parent)
    : IVisionAlgoService(parent)
{
}

void VisionAlgoServiceImpl::initialize()
{
    try {
        m_ready = !common::algorithm::AlgorithmFactory::Instance().List().empty();
    } catch (const std::exception &ex) {
        LOG_ERROR("[VisionAlgo] 服务初始化异常：{}", ex.what());
        m_ready = false;
    } catch (...) {
        LOG_ERROR("[VisionAlgo] 服务初始化发生未知异常");
        m_ready = false;
    }
}

bool VisionAlgoServiceImpl::isReady() const
{
    return m_ready;
}

bool VisionAlgoServiceImpl::isUnitAvailable(const std::string &unitName) const
{
    if (!m_ready || unitName.empty())
        return false;
    return common::algorithm::AlgorithmFactory::Instance().Contains(unitName);
}

common::errors::ErrorPtr VisionAlgoServiceImpl::execute(const std::string &unitName,
                                                        common::algorithm::AlgorithmExecuteData &data)
{
    if (!m_ready)
        return algoServiceNotReady();
    if (unitName.empty())
        return algoInvalidArgument("vision_algo.error.unit_name_param_null");

    const auto algo = common::algorithm::AlgorithmFactory::Instance().Create(unitName);
    if (!algo)
        return algoUnitNotAvailable(unitName.c_str());
    return algo->Execute(data);
}

std::future<common::errors::ErrorPtr> VisionAlgoServiceImpl::executeAsync(
    const std::string &unitName, common::algorithm::AlgorithmExecuteData *data)
{
    return std::async(std::launch::async, [this, unitName, data]() {
        if (!data)
            return algoInvalidArgument("vision_algo.error.param_null");
        return execute(unitName, *data);
    });
}

QString startupIssue()
{
    if (auto *svc = service()) {
        if (!svc->isReady())
            return tr("vision_algo.error.service_not_ready");

        QStringList issues;
        if (!svc->isUnitAvailable(kUnitDieBondingChipDetect))
            issues << tr("vision_algo.error.die_bonding_unit_not_registered");

        if (!queryModelSdkReady(svc)) {
            const QString detail = queryModelSdkDetail(svc);
            if (!detail.isEmpty())
                issues << detail;
        }
        if (!queryMarkerSdkReady(svc)) {
            const QString detail = queryMarkerSdkDetail(svc);
            if (!detail.isEmpty())
                issues << detail;
        }
        return issues.isEmpty() ? QString() : issues.join(QLatin1Char('\n'));
    }

    using ExtensionSystem::PluginManager;
    using ExtensionSystem::PluginSpec;
    for (PluginSpec *spec : PluginManager::plugins()) {
        if (spec->name() == QLatin1String("VisionAlgo")) {
            if (spec->hasError())
                return tr("vision_algo.error.plugin_load_failed").arg(spec->errorString());
            break;
        }
    }
    return tr("vision_algo.error.plugin_not_loaded");
}

QString modelSdkIssue()
{
    auto *svc = service();
    if (!svc) {
        const QString issue = startupIssue();
        return issue.isEmpty() ? QString() : appendLines(tr("vision_algo.model.sdk_missing"), issue);
    }
    if (svc->isReady() && queryModelSdkReady(svc))
        return {};
    const QString detail =
        svc->isReady() ? queryModelSdkDetail(svc) : tr("vision_algo.error.service_not_ready");
    return appendLines(tr("vision_algo.model.sdk_missing"), detail);
}

QString markModelSdkIssue()
{
    auto *svc = service();
    if (!svc) {
        const QString issue = startupIssue();
        return issue.isEmpty() ? QString() : appendLines(tr("vision_algo.model.sdk_missing"), issue);
    }
    if (svc->isReady() && queryModelSdkReady(svc) && queryMarkerSdkReady(svc))
        return {};

    QStringList details;
    if (!svc->isReady()) {
        details << tr("vision_algo.error.service_not_ready");
    } else {
        if (!queryModelSdkReady(svc)) {
            const QString d = queryModelSdkDetail(svc);
            if (!d.isEmpty())
                details << d;
        }
        if (!queryMarkerSdkReady(svc)) {
            const QString d = queryMarkerSdkDetail(svc);
            if (!d.isEmpty())
                details << d;
        }
    }
    return appendLines(tr("vision_algo.model.sdk_missing"), details.join(QLatin1Char('\n')));
}

} // namespace vision_algo
