/**
 * @file system_config_view.cpp
 * @brief 系统配置容器底栏保存按钮
 */

#include "system_config/system_config_view.h"

#include "system_config/system_config_general_view.h"
#include "system_config/system_config_switch_view.h"
#include "system_config/system_config_threshold_view.h"
#include "system_config_view_model.h"

#include "component/controls/button/themed_push_button.h"
#include "component/dialog/themed_message_box.h"
#include "databinding.h"

#include "uibase/examplepages.h"
#include "uibase/languagemanager.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

void showSaveResultMessage(QWidget *parent, bool success)
{
    ui::ThemedMessageBox::information(
        parent,
        TR("aoi.system.action.save"),
        TR(success ? QStringLiteral("aoi.system.action.saved")
                   : QStringLiteral("aoi.system.action.save_failed")),
        QString(),
        success ? ui::ThemedMessageBoxIcon::Information : ui::ThemedMessageBoxIcon::Warning);
}

} // namespace

// ── 底栏保存栏挂载 ──

void mountSystemConfigSaveBar(PageWithSubNav *container, SystemConfigViewModel *viewModel,
                              SystemConfigSwitchView *switchView,
                              SystemConfigGeneralView *generalView,
                              SystemConfigThresholdView *thresholdView)
{
    if (!container || !viewModel)
        return;

    auto *hLayout = qobject_cast<QHBoxLayout *>(container->layout());
    if (!hLayout || hLayout->count() < 2)
        return;

    QWidget *const navBar      = hLayout->itemAt(0)->widget();
    QWidget *const contentArea = hLayout->itemAt(1)->widget();
    hLayout->removeWidget(navBar);
    hLayout->removeWidget(contentArea);
    delete hLayout;

    auto *body = new QWidget(container);
    auto *bodyH = new QHBoxLayout(body);
    bodyH->setContentsMargins(0, 0, 0, 0);
    bodyH->setSpacing(0);
    bodyH->addWidget(navBar);
    bodyH->addWidget(contentArea, 1);

    auto *footer = new QWidget(container);
    footer->setObjectName(QStringLiteral("systemConfigFooter"));
    auto *footerH = new QHBoxLayout(footer);
    footerH->setContentsMargins(0, 8, 0, 0);
    footerH->addStretch();
    auto *saveBtn =
        makePushButton(TR("aoi.system.action.save"), PushButtonVariant::Primary, footer);
    footerH->addWidget(saveBtn);

    auto *root = new QVBoxLayout(container);
    root->setContentsMargins(18, 14, 18, 14);
    root->setSpacing(0);
    root->addWidget(body, 1);
    root->addWidget(footer, 0);

    const auto flushSources = [switchView, generalView, thresholdView]() {
        if (switchView)
            switchView->updateAllSources();
        if (generalView)
            generalView->updateAllSources();
        if (thresholdView)
            thresholdView->updateAllSources();
    };

    DataBinding::bindCommand(viewModel->saveCommand(), saveBtn, flushSources);

    QObject::connect(viewModel, &SystemConfigViewModel::saveSucceeded, container,
                     [container]() { showSaveResultMessage(container, true); });
    QObject::connect(viewModel, &SystemConfigViewModel::saveFailed, container,
                     [container]() { showSaveResultMessage(container, false); });

    QObject::connect(LM, &LanguageManager::languageChanged, container,
                     [saveBtn](LanguageManager::Language) {
                         saveBtn->setText(TR("aoi.system.action.save"));
                     });
}

} // namespace aoi
