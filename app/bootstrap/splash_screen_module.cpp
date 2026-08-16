#include "splash_screen_module.h"

#include "bootstrap_context.h"
#include "bootstrap_config.h"
#include "../splashscreen.h"

#include <Config.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QScreen>

namespace {

constexpr int kSplashWidth = 596;
constexpr int kSplashHeight = 157;

/// 绝对路径 / qrc（:/）原样返回；相对路径拼到 exe 目录（无 argv0 时用 applicationDirPath）。
QString resolveRuntimeRelativePath(const QString &path, const char *argv0)
{
    if (path.isEmpty())
        return {};
    if (path.startsWith(QStringLiteral(":/")))
        return path;

    const QFileInfo fi(path);
    if (fi.isAbsolute())
        return QDir::cleanPath(path);

    QString exeDir;
    if (argv0 != nullptr && argv0[0] != '\0')
        exeDir = QFileInfo(QString::fromLocal8Bit(argv0)).absolutePath();
    if (exeDir.isEmpty())
        exeDir = QCoreApplication::applicationDirPath();
    return QDir::cleanPath(QDir(exeDir).filePath(path));
}

bool splashEnabledFromConfig(const BootstrapConfig *cfg)
{
    if (!cfg)
        return false;
    if (config::ConfigManager::isLoaded())
        return config::ConfigManager::get<bool>("splash.enabled", cfg->splash.enabled);
    return cfg->splash.enabled;
}

QString splashImagePathFromConfig(const BootstrapConfig *cfg)
{
    QString path = cfg ? cfg->splash.imagePath : QString();
    if (config::ConfigManager::isLoaded()) {
        path = QString::fromStdString(config::ConfigManager::get<std::string>(
            "splash.imagePath", path.toStdString()));
    }
    return path;
}

bool splashImageFileExists(const QString &path)
{
    if (path.isEmpty())
        return false;
    if (path.startsWith(QStringLiteral(":/")))
        return QFile::exists(path);
    const QFileInfo fi(path);
    return fi.isFile() && fi.exists();
}

QPixmap loadPixmapFromPath(const QString &path)
{
    if (path.isEmpty())
        return {};
    QPixmap pixmap(path);
    if (!pixmap.isNull())
        qDebug() << "[bootstrap] splash image:" << path;
    return pixmap;
}

QPixmap defaultSplashPixmap(const QString &appName)
{
    QPixmap canvas(kSplashWidth, kSplashHeight);
    canvas.fill(Qt::white);

    QPainter painter(&canvas);
    QFont font(QStringLiteral("Microsoft YaHei"), 18, QFont::Normal);
    if (!font.exactMatch()) {
        font = QFont();
        font.setPointSize(18);
    }
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(canvas.rect(), Qt::AlignCenter, appName.isEmpty() ? QStringLiteral("Loading...") : appName);
    return canvas;
}

QPixmap resolveSplashPixmap(const BootstrapConfig *cfg, const char *argv0)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString imagePathRaw = splashImagePathFromConfig(cfg);

    if (!imagePathRaw.isEmpty()) {
        const QString configuredPath = resolveRuntimeRelativePath(imagePathRaw, argv0);
        if (!splashImageFileExists(configuredPath)) {
            qWarning() << "[bootstrap] SplashScreenModule: 配置的启动图不存在:"
                       << configuredPath;
        } else {
            const QPixmap pixmap = loadPixmapFromPath(configuredPath);
            if (!pixmap.isNull())
                return pixmap;
            qWarning() << "[bootstrap] SplashScreenModule: 配置的启动图无法解码:"
                       << configuredPath;
        }
    }

    const QStringList fallbackCandidates = {
        QDir(appDir).filePath(QStringLiteral("config/splash.png")),
        QDir(appDir).filePath(QStringLiteral("config/splash.ico")),
        QStringLiteral(":/resources/image/app_splash.ico"),
    };

    for (const QString &path : fallbackCandidates) {
        const QPixmap pixmap = loadPixmapFromPath(path);
        if (!pixmap.isNull())
            return pixmap;
    }

    qWarning() << "[bootstrap] SplashScreenModule: no splash image found, using default canvas";
    return defaultSplashPixmap(cfg ? cfg->applicationName : QString());
}

} // namespace

SplashScreenModule::SplashScreenModule() = default;

SplashScreenModule::~SplashScreenModule() = default;

QString SplashScreenModule::name() const
{
    return QStringLiteral("SplashScreen");
}

int SplashScreenModule::priority() const
{
    return 9;
}

bool SplashScreenModule::onBoot(BootstrapContext &ctx)
{
    const BootstrapConfig *cfg = ctx.config;
    if (!splashEnabledFromConfig(cfg))
        return true;

    QApplication *app = ctx.application;
    if (!app) {
        qWarning() << "[bootstrap] SplashScreenModule: missing QApplication";
        return true;
    }

    QScreen *screen = QApplication::primaryScreen();
    if (!screen) {
        qWarning() << "[bootstrap] SplashScreenModule: no primary screen";
        return true;
    }

    splash_ = std::make_unique<SplashScreen>(screen);
    ctx.splashScreen = splash_.get();

    const char *argv0 =
        (ctx.argv != nullptr && ctx.argc != nullptr && *ctx.argc > 0) ? ctx.argv[0] : nullptr;
    splash_->setOriginalPixmap(resolveSplashPixmap(cfg, argv0));
    splash_->setWindowFlags(splash_->windowFlags() | Qt::WindowStaysOnTopHint);
    splash_->show();
    splash_->raise();
    splash_->activateWindow();
    app->processEvents();

    ctx.releaseSplashScreen = [this, &ctx]() { releaseSplash(ctx); };
    return true;
}

void SplashScreenModule::releaseSplash(BootstrapContext &ctx)
{
    if (ctx.splashScreen == splash_.get())
        ctx.splashScreen = nullptr;
    splash_.reset();
}

void SplashScreenModule::onShutdown(BootstrapContext &ctx)
{
    ctx.releaseSplashScreen = nullptr;
    releaseSplash(ctx);
}
