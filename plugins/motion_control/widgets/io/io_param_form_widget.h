#pragma once

/**
 * @file io_param_form_widget.h
 * @brief 单路 IO 参数卡片控件
 *
 * 用于 Motion IO 设置页网格中的单个 DI/DO 条目，展示与编辑 `IoConfigData`。
 *
 * ## 视觉结构（自上而下）
 * ```
 * m_cardFrame（透明底 + 圆角描边）
 * ├── m_headerFrame（surfaceElevated：端口前缀 / 名称 / 运行态指示灯）
 * └── m_contentSlot（参数区外层，可整体隐藏）
 *     └── m_contentFrame（t.background：卡名 / 有效电平 / 端口 / 扩展卡 / 扩展卡号表单）
 * ```
 *
 * ## 权限与交互
 * - `setDetailsVisible(false)`：无 IoEdit 权限时隐藏参数区；有 IoEdit 时可编辑并可保存
 * - `setOutputClickEnabled(true)`：输出 IO 允许点击标题栏，经 `readIo`/`writeIo` 翻转电平
 * - 点击卡片 emit cardSelected，供 MotionIoSettingsView 底部「删除 IO」使用
 *
 * ## 尺寸策略
 * 卡片纵向 `Fixed`，高度由 `syncCardLayout()` 按固定行高公式计算，避免网格行 stretch 撑出空白。
 *
 * @see MotionIoSettingsView
 * @see IMotionControlService::ioConfig / setIoConfig / readIo / writeIo
 */

#include <imotion_control_service.h>
#include <motion_control_types.h>

#include <QWidget>

class QFrame;
class QGridLayout;
class QVBoxLayout;

namespace ui {
class ThemedComboBox;
class ThemedFormLabel;
class ThemedGlassIndicatorDot;
class ThemedRadioButton;
class ThemedSpinBox;
class PillSwitch;
} // namespace ui

namespace motion_control_plugin {

class IoParamFormWidget : public QWidget
{
    Q_OBJECT

signals:
    /** 用户点击卡片（选中）；输出 IO 标题栏翻转时不触发 */
    void cardSelected();

public:
    explicit IoParamFormWidget(IMotionControlService *service, QWidget *parent = nullptr);

    /** 填充运动卡名下拉选项（来自 `IMotionControlService::motionCardNames`） */
    void setCardNameOptions(const QStringList &cardNames);

    /** 从服务配置加载到表单，并同步标题/权限态/布局 */
    void loadConfig(const IoConfigData &config);

    /** 收集当前表单值为 `IoConfigData`（logic：0=低有效，1=高有效） */
    IoConfigData collectConfig() const;

    QString ioName() const { return m_config.name; }
    bool isOutput() const { return m_config.isOutput; }

    /** 刷新标题栏右侧指示灯（active=true 时使用 success 色） */
    void refreshRuntimeState(bool active);

    /** 显示/隐藏参数区；隐藏时通过 syncCardLayout 收缩卡片高度 */
    void setDetailsVisible(bool visible);

    /** 是否允许输出 IO 点击标题栏切换（需 IoWrite 权限） */
    void setOutputClickEnabled(bool enabled);

    /** 选中高亮（底部「删除 IO」作用于当前选中卡片） */
    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    /** 处理 m_headerFrame 左键释放 → toggleOutput */
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void buildUi();
    void buildHeaderFrame(QVBoxLayout *cardLay);
    void buildContentFrame(QVBoxLayout *cardLay);
    void retranslateUi();
    /** 应用 QSS：外框透明、标题 surfaceElevated、内容 t.background */
    void refreshCardStyle();

    /** 主题/样式表刷新后重算列宽与行高（字体/QSS 变化后 sizeHint 可能变小导致挤压） */
    void onThemeRefresh();

    /** 重算控件列宽（FontMetrics + 行 minimumHeight）并同步卡片尺寸 */
    void syncLevelRowLayout();

    /** 扩展卡未勾选时隐藏扩展卡号行并收缩卡片高度 */
    void syncExtendNodeVisible();

    /**
     * 按 kFormRowCount × kFormRowHeight 公式固定 contentFrame / contentSlot / cardFrame 高度。
     * 折叠参数区（m_detailsVisible=false）时不计入 contentSlot 高度。
     */
    void syncCardLayout();

    void updatePortHeader();
    void updateHeaderInteraction();
    void toggleOutput();
    /** 标题左侧端口前缀，如 "DI: 3" / "DO:12"（单位数前补空格对齐） */
    static QString formatPortHeader(bool isOutput, int port);
    void bindCardCombo(const QString &current);
    void refreshSelectionStyle();

    IMotionControlService *m_service = nullptr;
    IoConfigData           m_config;
    QStringList            m_cardNames;
    bool                   m_detailsVisible     = true;
    bool                   m_outputClickEnabled = false;
    bool                   m_selected           = false;

    // --- 卡片骨架 ---
    QFrame  *m_cardFrame   = nullptr;
    QFrame  *m_headerFrame = nullptr;
    QWidget *m_contentSlot = nullptr;  ///< 包裹参数区，便于 setDetailsVisible 整体显隐
    QFrame  *m_contentFrame = nullptr;
    QGridLayout *m_formGrid = nullptr; ///< 表单网格；行 minimumHeight 防止主题切换后间距被压扁

    // --- 标题栏 ---
    ui::ThemedFormLabel         *m_portHeaderLbl = nullptr;
    ui::ThemedFormLabel         *m_titleLbl      = nullptr;
    ui::ThemedGlassIndicatorDot *m_stateDot     = nullptr;

    // --- 参数表单（双列：标签 | 控件）---
    ui::ThemedFormLabel   *m_cardLbl    = nullptr;
    ui::ThemedComboBox    *m_cardCombo  = nullptr;
    ui::ThemedFormLabel   *m_levelLbl   = nullptr;
    ui::ThemedRadioButton *m_lowActive  = nullptr;
    ui::ThemedRadioButton *m_highActive = nullptr;
    QWidget               *m_levelWrap  = nullptr; ///< 有效电平单选行容器（固定宽高）
    ui::ThemedFormLabel   *m_portLbl    = nullptr;
    ui::ThemedSpinBox     *m_portSpin   = nullptr;
    ui::ThemedFormLabel   *m_extendLbl   = nullptr;
    ui::PillSwitch        *m_extendSwitch = nullptr;
    ui::ThemedFormLabel   *m_nodeLbl     = nullptr;
    ui::ThemedSpinBox     *m_nodeSpin     = nullptr;
};

} // namespace motion_control_plugin
