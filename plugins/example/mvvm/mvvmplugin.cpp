#include "mvvmplugin.h"

#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"

// MVVM 示例页面
#include "userview.h"
#include "deviceview.h"

#include <QDebug>

using namespace ExtensionSystem;

bool MvvmPlugin::initialize(QString *errorString)
{
    Q_UNUSED(errorString)

    IMainWindowService *svc = PluginManager::getObject<IMainWindowService>();
    if (!svc) {
        if (errorString)
            *errorString = "MvvmPlugin: IMainWindowService not found.";
        qWarning() << "[MvvmPlugin] IMainWindowService not found!";
        return false;
    }

    // ── 注册 MVVM 基础示例页面（UserView，order=50）──────────────────────────
    auto *userView = new UserView();
    svc->addPage("mvvm", "nav.mvvm", userView, 50);
    qDebug() << "[MvvmPlugin] UserView page registered.";

    // ── 注册 MVVM 增强示例页面（DeviceView，order=51）──────────────────────────
    // 演示：结构体绑定 / QSlider / QDoubleSpinBox / QProgressBar /
    //       QDateEdit / QTimeEdit / QRadioButton / bindEnabled / bindVisible
    auto *deviceView = new DeviceView();
    svc->addPage("mvvm_device", "nav.mvvm_device", deviceView, 51);
    qDebug() << "[MvvmPlugin] DeviceView (enhanced MVVM demo) page registered.";

    return true;
}
