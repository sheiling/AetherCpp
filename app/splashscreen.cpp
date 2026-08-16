// ============================================================
// splashscreen.cpp — 应用启动画面（QSplashScreen 子类）
//
// 【职责】
//   - 显示启动图（内置/配置/回退路径，见 SplashScreenModule）
//   - 订阅 MessageCenter EVENT_SPLASH_MESSAGE，底部动态刷新加载提示
//
// 【显示模式】按图片像素原尺寸（app.toml splash.imagePath）
//
// 【绘制层次】（drawContents，自底向上）
//   1. pixmap() 启动图
//   2. 状态文字半透明白底（深色图下保证黑字可读）
//   3. 底部居中状态文字
//
// 【生命周期】
//   SplashScreenModule::onBoot 创建 → PluginLoaderModule 加载完成后 finish(mainWindow)
//   右上角「×」可立即中止启动（std::exit）；插件加载循环内 pump 事件以响应点击
//   析构前 Detach MessageCenter，避免悬挂回调
// ============================================================

#include "splashscreen.h"

#include "messagecenter/message_center.h"
#include "messagecenter/message_define.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QResizeEvent>
#include <QVariantList>

#include <cstdlib>

namespace {

constexpr int kFontSize = 15;           ///< 底部状态文字字号
constexpr int kBottomMargin = 14;       ///< 文字距窗口底边留白
constexpr int kStatusBgHPadding = 10;   ///< 文字底衬水平内边距
constexpr int kStatusBgVPadding = 4;    ///< 文字底衬垂直内边距
constexpr int kStatusBgRadius = 4;      ///< 文字底衬圆角
constexpr int kAbortBtnSize     = 22;
constexpr int kAbortBtnMargin   = 6;

} // namespace

// ── 构造 / 析构：MessageCenter 订阅 ─────────────────────────────

SplashScreen::SplashScreen(QScreen *screen)
    : QSplashScreen(screen)
{
    // 各模块通过 EVENT_SPLASH_MESSAGE 推送加载进度文案（list[0] 为 QString）
    MessageCenter::Center::Attach(
        EVENT_SPLASH_MESSAGE, this, SLOT(slot_message(const QVariantList &)));
    setupAbortButton();
}

SplashScreen::~SplashScreen()
{
    if (QCoreApplication::instance()) {
        MessageCenter::Center::Detach(
            EVENT_SPLASH_MESSAGE, this, SLOT(slot_message(const QVariantList &)));
    }
}

// ── 启动图设置 ──────────────────────────────────────────────────

void SplashScreen::setOriginalPixmap(const QPixmap &source)
{
    if (source.isNull())
        return;
    // 窗口尺寸 = 图片尺寸；drawContents 直接绘制整图
    setFixedSize(source.size());
    setPixmap(source);
    positionAbortButton();
}

void SplashScreen::setupAbortButton()
{
    abortBtn_ = new QPushButton(QStringLiteral("\u00D7"), this);
    abortBtn_->setObjectName(QStringLiteral("splashAbortBtn"));
    abortBtn_->setFixedSize(kAbortBtnSize, kAbortBtnSize);
    abortBtn_->setFlat(true);
    abortBtn_->setCursor(Qt::PointingHandCursor);
    abortBtn_->setToolTip(QStringLiteral("退出启动"));
    abortBtn_->setStyleSheet(QStringLiteral(
        "QPushButton#splashAbortBtn {"
        "  background-color: rgba(0, 0, 0, 90);"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 11px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  padding: 0px;"
        "}"
        "QPushButton#splashAbortBtn:hover {"
        "  background-color: rgba(180, 40, 40, 220);"
        "}"));
    abortBtn_->raise();
    connect(abortBtn_, &QPushButton::clicked, this, &SplashScreen::onAbortClicked);
    positionAbortButton();
}

void SplashScreen::positionAbortButton()
{
    if (!abortBtn_)
        return;
    abortBtn_->move(width() - kAbortBtnSize - kAbortBtnMargin, kAbortBtnMargin);
    abortBtn_->raise();
}

void SplashScreen::resizeEvent(QResizeEvent *event)
{
    QSplashScreen::resizeEvent(event);
    positionAbortButton();
}

void SplashScreen::onAbortClicked()
{
    qWarning() << "[bootstrap] SplashScreen: user aborted startup";
    close();
    std::exit(1);
}

// ── 绘制：启动图 + 底部状态条 ───────────────────────────────────

void SplashScreen::drawContents(QPainter *painter)
{
    if (!painter)
        return;

    const QPixmap pm = pixmap();
    if (!pm.isNull())
        painter->drawPixmap(0, 0, pm);

    if (statusText_.isEmpty())
        return;

    QFont font(QStringLiteral("Microsoft YaHei"), kFontSize, QFont::Normal);
    if (!font.exactMatch()) {
        font = QFont();
        font.setPointSize(kFontSize);
    }
    painter->setFont(font);

    // 布局区：底部 kBottomMargin 之上，文字水平居中
    const QRect textLayoutRect(0, 0, width(), height() - kBottomMargin);
    const QFontMetrics fm(font);
    const QRect textBox =
        fm.boundingRect(textLayoutRect, Qt::AlignBottom | Qt::AlignHCenter, statusText_);

    // 按文字包围盒加 padding 绘制底衬，并裁剪到窗口内
    QRect bgRect = textBox.adjusted(-kStatusBgHPadding, -kStatusBgVPadding,
                                    kStatusBgHPadding, kStatusBgVPadding);
    bgRect = bgRect.intersected(QRect(0, 0, width(), height()));

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(255, 255, 255, 220)); // 偏白半透明，深色启动图下保证黑字可读
    painter->drawRoundedRect(bgRect, kStatusBgRadius, kStatusBgRadius);

    painter->setPen(Qt::black);
    painter->drawText(textLayoutRect, Qt::AlignBottom | Qt::AlignHCenter, statusText_);
}

// ── MessageCenter 回调 ──────────────────────────────────────────

void SplashScreen::slot_message(const QVariantList &list)
{
    if (list.isEmpty() || !list[0].canConvert<QString>())
        return;

    statusText_ = list[0].toString();
    repaint();

    // 启动阶段需立即刷新 UI，避免长时间无响应
    if (QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance()))
        app->processEvents();
}
