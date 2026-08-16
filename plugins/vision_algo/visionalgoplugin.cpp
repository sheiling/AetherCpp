#include "visionalgoplugin.h"

#include "service/vision_algo_service_impl.h"

#include "src/core/vision_algo_error_codes.h"

#include <extensionsystem/pluginmanager.h>
#include <uibase/languagemanager.h>

#include <Logger.h>

#include <QDebug>

#include <exception>

namespace {
/** 进程启动时注册 VisionAlgo 错误码到 common::errors 体系 */
const bool kVisionAlgoCodersRegistered =
    ::common::errors::algo_module::RegisterVisionAlgoCoders();
} // namespace

VisionAlgoPlugin::~VisionAlgoPlugin()
{
    // 服务由 PluginManager 管理生命周期时可在此 removeObject；当前随进程退出释放
}

bool VisionAlgoPlugin::initialize(QString *errorString)
{
    LM->registerPluginTranslator(QStringLiteral("vision_algo"), QStringLiteral("vision_algo"));

    try {
        m_service = new vision_algo::VisionAlgoServiceImpl();
        m_service->initialize();
        ExtensionSystem::PluginManager::addObject(m_service);
    } catch (const std::exception &ex) {
        if (errorString)
            *errorString = QString::fromUtf8(ex.what());
        LOG_ERROR("[VisionAlgoPlugin] 初始化失败：{}", ex.what());
        return false;
    } catch (...) {
        if (errorString)
            *errorString = QStringLiteral("VisionAlgoPlugin: unknown exception");
        LOG_ERROR("[VisionAlgoPlugin] 初始化发生未知异常");
        return false;
    }

    LOG_INFO("[VisionAlgoPlugin] IVisionAlgoService 已注册，就绪={}", m_service->isReady());
    return true;
}

void VisionAlgoPlugin::extensionsInitialized()
{
}
