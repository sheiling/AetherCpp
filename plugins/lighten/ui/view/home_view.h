#pragma once

/**
 * @file home_view.h
 * @brief 点亮检测首页视图（设备控制、KPI、图像预览、检测进度、结果快照、运行日志）
 */

#include "pagewidget.h"

#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QTableWidget>
#include <QTextEdit>
#include <QStackedWidget>
#include <QGroupBox>
#include <QVector>
#include <QVariantList>
#include <QString>
#include <QStringList>

#include <unordered_map>

#include "lighten_types.h"

#include "home_view_model.h"
#include "system_config_model.h"
#include "recipe_model.h"

#include <string>

class IGraphicsVew;

namespace lighten {

class HomeViewModel;

/**
 * @brief 首页视图
 *
 * 布局（横向）：
 *   ┌─ Sidebar 200px ─┬─ Main Content ──────────────────────────────────┐
 *   │ 设备控制 9 buttons│ KPI Bar (10 cards)                             │
 *   │ ─────────────── │ ┌──────────────┬──────────────────────────────┐│
 *   │ 硬件状态 5 rows  │ │ 图像预览面板 │ 检测项进度(2列) + 结果快照   ││
 *   │                 │ ├──────────────┴──────────────────────────────┤│
 *   │                 │ │ 运行日志                                     ││
 *   │                 │ └──────────────────────────────────────────────┘│
 *   └─────────────────┴──────────────────────────────────────────────────┘
 */
class HomeView : public PageWidget
{
    Q_OBJECT

public:
    explicit HomeView(QWidget *parent = nullptr);
    ~HomeView() override;

    /** 运行时更新 KPI 卡片显示值 */
    void updateKpi(int index, const QString &value);
    /** 运行时更新检测项进度状态文本 */
    void updateProgressStatus(int slotIndex, const QString &statusText);
    /** 向运行日志区追加一条日志 */
    void appendLog(const QString &text, const QString &level = "INFO");

protected:
    void onLoad() override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void slot_img_path(const QVariantList &args);
    void slot_camera_conn_state(const QVariantList &args);
    void slot_flash_detect_result(const QVariantList &args);
    void slot_black_screen_detect_result(const QVariantList &args);
    void slot_black_screen_light_detect_result(const QVariantList &args);
    void slot_lighten_defect_count_result(const QVariantList &args);
    void slot_lighten_detect_progress(const QVariantList &args);
    void slot_motion_conn_state(const QVariantList &args);
    void slot_scanner_conn_state(const QVariantList &args);
    void slot_general_settings_changed(const QVariantList &args);
    void slot_system_config_saved(const QVariantList &args);
    void slot_production_kpi_updated(const QVariantList &args);
    /** 首页「初始化」：重置进度/快照并刷新检测工步 FlowContext GLOBAL 配置 */
    void slot_initialize();

private:
    QString previewImagePath() const;
    void applyPreviewImage();
    void applyResultTableWidths();
    void applyRuntimeToUi(const HomeRuntimeState &rt);
    void     applyHomeTheme();
    QWidget *buildSidebar();
    QWidget *buildMainArea();
    void     buildHardwareStatusRows();
    void     refreshHardwareStatus();
    void     appendHardwareConnLog(const QString &deviceName, bool connected);
    void     attachHardwareEvents();
    void     detachHardwareEvents();
    void     buildKpiBar(QVBoxLayout *root);
    void     buildContentArea(QVBoxLayout *root);
    QWidget *buildImagePanel();
    QWidget *buildRightStack();
    QWidget *buildProgressCard();
    QWidget *buildResultCard();
    QWidget *buildLogCard();
    void     setupViewModel();
    /** 重置「当前检测结果快照」为 defaultEmpty 初始态 */
    void     resetInspectionSnapshot();
    /** 重置「检测项进度」：全部状态为未开始 */
    void     resetDetectProgress();
    /** 按系统配置检测项开关显示/隐藏进度行并紧凑重排 */
    void     refreshDetectProgressLayout();

    // ── 检测工步运行时配置（FlowContext GLOBAL）──

    /** 从 SystemConfig 读取 Flash 检测总开关，写入 KEY_FLASH_STEP_CONFIG */
    bool setFlashDetectStepConfig(const SystemConfig &cfg);

    /** 从 SystemConfig 读取「是否扫码」，写入 KEY_SCAN_STEP_CONFIG */
    bool setScanStepConfig(const SystemConfig &cfg);

    /** 从 SystemConfig 读取「上游对接」，写入 KEY_UPSTREAM_DOCK_STEP_CONFIG */
    bool setUpstreamDockStepConfig(const SystemConfig &cfg);

    /** 从 SystemConfig 读取「下游对接」，写入 KEY_DOWNSTREAM_DOCK_STEP_CONFIG */
    bool setDownstreamDockStepConfig(const SystemConfig &cfg);

    /** 从 SystemConfig / RecipeDocument 读取检测开关与算法参数，写入 KEY_LIGHT_DETECT_STEP_CONFIG */
    bool setLightDetectStepConfig(const SystemConfig &cfg, const RecipeDocument &recipe);

    /** 从 SystemConfig / RecipeDocument 读取判定阈值、特殊规则与 panelCoord，写入 KEY_RESULT_SUMMARY_STEP_CONFIG */
    bool setResultSummaryStepConfig(const SystemConfig &cfg, const RecipeDocument &recipe);

    /** 将默认配方 ID 写入 KEY_PRODUCT_MODEL（FlowContext GLOBAL） */
    bool setProductModelFlowContext(const QString &defaultRecipeId);

    /** 从 SystemConfig / 默认 Recipe 加载并写入各工步 GLOBAL 运行时配置（仅首页「初始化」调用） */
    bool buildStepConfigFromSystemConfig();
    /** 将 lighten_production_kpi 重置为默认值并通知界面刷新 */
    void clearProductionKpi();

    HomeViewModel *m_viewModel = nullptr;

    // KPI value labels (10 items)
    QVector<QLabel *>       m_kpiValues;

    // Image preview
    IGraphicsVew           *m_graphicsView = nullptr;
    QStackedWidget         *m_previewStack = nullptr;
    QString                 m_img_path;
    bool                    m_bOriginal = true;
    ColorChannel            m_colorChannel = ColorChannel::Red;
    QString                 m_camePath;
    DetectItemKind          m_detectItemKind = DetectItemKind::DeadLed;

    // Detection progress (21 slots, visibility from system config R/G/B columns)
    QGridLayout            *m_progressGrid = nullptr;
    QVector<QWidget *>      m_progressCells;
    QVector<QLabel *>       m_progressStatus;

    // Result snapshot table
    QTableWidget           *m_resultTable = nullptr;
    QLabel                 *m_resultInfoLabel = nullptr;

    // Run log
    QTextEdit              *m_logEdit = nullptr;

    /** DeviceManager.Camera：显示名 → path（来自 device_config.json） */
    std::unordered_map<std::string, std::string> m_cameraMap;

    enum class HwDeviceKind {
        Camera,
        Motion,
        Scanner,
    };

    struct HwStatusRow {
        HwDeviceKind kind = HwDeviceKind::Camera;
        QString      displayName;
        QString      deviceKey;
        QLabel      *dot = nullptr;
        QLabel      *status = nullptr;
    };

    QGroupBox           *m_hwBox = nullptr;
    QVBoxLayout         *m_hwLayout = nullptr;
    QVector<HwStatusRow> m_hwRows;
    QVector<QString>     m_motionCardNames;

};

} // namespace lighten
