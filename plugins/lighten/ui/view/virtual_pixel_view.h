#pragma once

/**
 * @file virtual_pixel_view.h
 * @brief 虚拟像素视图（观察预览、三通道配置、映射规则）
 */

#include "pagewidget.h"
#include "virtual_pixel_model.h"

#include <QLabel>
#include <QString>
#include <QTableWidget>
#include <QTextEdit>
#include <QStackedWidget>
#include <QVector>
#include <QPushButton>
#include <QPointer>
#include <QVariantList>

class QDialog;
class IGraphicsVew;
class QButtonGroup;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

namespace lighten {

class VirtualPixelViewModel;

/** @deprecated 使用 VirtualPixelChannelConfig（model 层） */
using VpChannelCfgSnap = VirtualPixelChannelConfig;

/**
 * @brief 虚拟像素视图
 *
 * 布局（左右分割）：
 *   ┌─ 观察与结果输出 (left, ~55%) ─────┬─ 虚拟像素配置 (right, ~45%) ──┐
 *   │  相机选择 + 操作按钮 toolbar       │  产品配方选择                  │
 *   │  [图像预览画布]                    │  R/G/B 通道 tabs               │
 *   │  结果输出表格                      │  排列方式示例                   │
 *   │  运行日志                          │  灯点行列（行/列 Spin 同行）     │
 *   │                                   │  坐标映射规则                   │
 *   │                                   │                                │
 *   └───────────────────────────────────┴─────────────────────────────────┘
 */
class VirtualPixelView : public PageWidget
{
    Q_OBJECT

public:
    explicit VirtualPixelView(QWidget *parent = nullptr);
    ~VirtualPixelView() override;

    /** 向运行日志区追加一条日志 */
    void appendLog(const QString &text, const QString &level = "INFO");

protected:
    void onLoad() override;

private slots:
    void slot_camera_img(const QVariantList &args);
    void slot_camera_state(const QVariantList &args);

private:
    void buildLayout();
    void setupViewModel();
    QWidget *buildObservePanel();
    QWidget *buildConfigPanel();
    /** 切换 R/G/B 时保存当前通道 UI 并载入目标通道数据 */
    void syncVpChannelSharedUi(int channelRgb);
    /** 将当前活动通道的 UI 写入 m_vpChannelCfg */
    void flushActiveChannelFromUi();
    /** 将界面状态写入 ViewModel document（保存前调用） */
    void syncUiToDocument();
    /** 从 ViewModel document 刷新界面 */
    void applyDocumentToUi();
    /** 居中无边框提示，1 秒后淡出关闭 */
    void showSaveSuccessTip();
    /** 同步产品配方下拉（配方配置页增删改后调用） */
    void refreshRecipeCombo();
    /** 实时预览开关操作失败时恢复按钮 checked 与文字 */
    void revertLiveToggleButton();
    /** 按当前选中相机的 GetCaptureState 同步实时预览/抓拍按钮 */
    void syncPreviewButtonFromCaptureState();

    VirtualPixelViewModel *m_viewModel = nullptr;
    QComboBox  *m_recipeCombo       = nullptr;
    QComboBox  *m_observeCamCombo   = nullptr;
    QPointer<QPushButton> m_liveToggleBtn = nullptr;
    QPointer<QPushButton> m_snapBtn       = nullptr;
    QPushButton *m_saveBtn          = nullptr;
    bool        m_applyingDocument  = false;
    QPointer<QDialog> m_saveSuccessTipDlg;

    QTableWidget  *m_resultTable  = nullptr;
    QTextEdit     *m_logEdit      = nullptr;
    IGraphicsVew  *m_graphicsView = nullptr;
    QStackedWidget *m_previewStack = nullptr;

    QStackedWidget *m_channelStack = nullptr;
    QVector<QPushButton *> m_channelBtns;

    QButtonGroup *m_observeColorGroup = nullptr;
    QButtonGroup *m_channelGroup     = nullptr;
    bool          m_syncingObserveConfigRgb = false;

    QButtonGroup *m_demoPickGroup = nullptr;
    QVector<QFrame *>     m_demoWrapFrames;
    QVector<QLineEdit *>  m_mapRuleEdits;
    QVector<QSpinBox *>   m_vpLampRowSpins;
    QVector<QSpinBox *>   m_vpLampColSpins;

    VirtualPixelChannelConfig m_vpChannelCfg[3]{};
    int              m_vpActiveCfgChannel = -1;
};

} // namespace lighten
