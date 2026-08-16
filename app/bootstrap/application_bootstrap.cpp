#include "application_bootstrap.h"
#include "bootstrap_context.h"
#include "i_bootstrap_module.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QIcon>

#include <algorithm>
#include <utility>

ApplicationBootstrap::ApplicationBootstrap(BootstrapConfig config)
    : config_(std::move(config))
{}

ApplicationBootstrap::~ApplicationBootstrap() = default;

void ApplicationBootstrap::addModule(std::unique_ptr<IBootstrapModule> module)
{
    modules_.push_back(std::move(module));
}

void ApplicationBootstrap::sortModules()
{
    std::sort(modules_.begin(), modules_.end(),
              [](const std::unique_ptr<IBootstrapModule> &a,
                 const std::unique_ptr<IBootstrapModule> &b) {
                  if (a->priority() != b->priority())
                      return a->priority() < b->priority();
                  return a->name() < b->name();
              });
}

static void shutdownModules(BootstrapContext &ctx,
                            std::vector<std::unique_ptr<IBootstrapModule>> &mods)
{
    for (auto it = mods.rbegin(); it != mods.rend(); ++it)
        (*it)->onShutdown(ctx);
}

int ApplicationBootstrap::run(int &argc, char **argv)
{
    sortModules();

    QApplication application(argc, argv);
    if (!config_.applicationName.isEmpty())
        QCoreApplication::setApplicationName(config_.applicationName);
    if (!config_.applicationVersion.isEmpty())
        QCoreApplication::setApplicationVersion(config_.applicationVersion);

    // 任务栏 / 窗口默认图标（与 resources/app.rc 中 IDI_ICON1 一致）
    application.setWindowIcon(
        QIcon(QStringLiteral(":/resources/image/short_logo.ico")));

    BootstrapContext ctx(&argc, argv, &config_);
    ctx.application = &application;

    for (size_t i = 0; i < modules_.size(); ++i) {
        if (!modules_[i]->onRegister(ctx)) {
            qWarning() << "[bootstrap] module" << modules_[i]->name()
                       << "onRegister failed, abort.";
            while (i > 0) {
                --i;
                modules_[i]->onShutdown(ctx);
            }
            return 1;
        }
    }

    for (const auto &m : modules_) {
        if (!m->onBoot(ctx)) {
            qWarning() << "[bootstrap] module" << m->name()
                       << "onBoot failed, abort.";
            if (ctx.releaseSplashScreen) {
                ctx.releaseSplashScreen();
                if (QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance()))
                    app->processEvents();
            }
            shutdownModules(ctx, modules_);
            return 1;
        }
    }

    const int exitCode = application.exec();

    shutdownModules(ctx, modules_);
    return exitCode;
}
