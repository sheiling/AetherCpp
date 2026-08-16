/**
 * @file home_view.cpp
 * @brief HomeView 布局构建、主题样式与检测工步配置绑定实现
 */

#include "home_view.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QFrame>
#include <QSizePolicy>
#include <QButtonGroup>
#include <QScrollArea>
#include <QHeaderView>
#include <QDateTime>
#include <QFont>
#include <QFontMetrics>
#include <QSet>
#include <QHash>
#include <QResizeEvent>
#include <QCoreApplication>
#include <QDir>
#include <QImage>
#include <QDebug>
#include <QMenu>
#include <utility>

#include "lighten_param_persist.h"
#include "home_model.h"

#include <common/core/device/deviceManager/device_manager.h>

#include "interface/igraphicsView.h"
#include "extensionsystem/pluginmanager.h"
#include "messagecenter/message_center.h"
#include "messagecenter/message_define.h"
#include "lightenplugin.h"
#include "system_config_model.h"
#include "recipe_model.h"

#include <interface/ilighten_production_kpi_service.h>

#include <scheduler/step/i_step.h>

#include <common/include/algorithm_data/lightendetector_data.h>
#include <common/utility/json_tool/reflector.h>

using namespace ExtensionSystem;
using namespace lighten;

// ── local helpers + 主题色（对齐目标稿）──────────────────────────────────────
/** HomeView 本地布局辅助、主题常量与工步绑定工具 */
namespace {

constexpr const char *kTextMuted  = "#9aa4b5";

// 首页「检测项进度」布局常量
constexpr int kProgressGridHSpacing = 28; ///< 同一行左右两个检测项间距
constexpr int kProgressGridVSpacing = 12; ///< 行与行之间间距
constexpr int kProgressTitleGap     = 32; ///< 标题「检测项进度」与首行检测项间距
constexpr int kProgressInnerSpacing = 8;
constexpr int kProgressStatusW      = 44;

template<typename T>
void setGlobalFlowContextVar(const std::string &key, T &&value)
{
    scheduler::FlowContext ctx;
    ctx.set(key, std::forward<T>(value), scheduler::VariableScope::GLOBAL);
}

QString homeProgressSlotLabel(int slot)
{
    DetectItemKind kind = DetectItemKind::DeadLed;
    int            colorIdx = -1;
    decodeHomeProgressSlot(slot, &kind, &colorIdx);
    if (colorIdx >= 0)
        return homeProgressItemName(kind, static_cast<ColorChannel>(colorIdx));
    return detectItemName(kind);
}

QString detectProgressStatusText(int statusInt)
{
    using common::algorithm::LightenDetectProgressStatus;
    switch (static_cast<LightenDetectProgressStatus>(statusInt)) {
    case LightenDetectProgressStatus::NotStarted:
        return QStringLiteral("未开始");
    case LightenDetectProgressStatus::InProgress:
        return QStringLiteral("进行中");
    case LightenDetectProgressStatus::Completed:
        return QStringLiteral("已完成");
    }
    return QStringLiteral("未开始");
}

const QStringList kDefectTypes = {
    "死灯", "暗灯", "亮灯", "串色", "串灯", "混色", "黑屏亮灯"
};

common::algorithm::DefectCountLimits defectCountLimitsFromSystemConfig(const SystemConfig &cfg)
{
    using common::algorithm::DefectCountLimitRange;
    using common::algorithm::DefectCountLimits;

    DefectCountLimits limits{};
    limits.dead_count.lower             = 0;
    limits.dead_count.upper             = 3;
    limits.dim_count.lower              = 0;
    limits.dim_count.upper              = 3;
    limits.bright_count.lower           = 0;
    limits.bright_count.upper           = 2;
    limits.color_variation_count.lower  = 0;
    limits.color_variation_count.upper  = 2;
    limits.string_light_count.lower     = 0;
    limits.string_light_count.upper     = 1;
    limits.mix_color_count.lower            = 0;
    limits.mix_color_count.upper            = 1;
    limits.total_defect_count.lower         = 0;
    limits.total_defect_count.upper         = 5;

    QHash<QString, const ThresholdRange *> byName;
    for (const ThresholdRange &t : cfg.thresholdRanges)
        byName.insert(t.name, &t);

    const auto apply = [&byName](DefectCountLimitRange &range, const QString &name) {
        const auto it = byName.constFind(name);
        if (it == byName.constEnd())
            return;
        range.lower = it.value()->lower;
        range.upper = it.value()->upper;
    };

    apply(limits.dead_count, QStringLiteral("死灯数量"));
    apply(limits.dim_count, QStringLiteral("暗灯数量"));
    apply(limits.bright_count, QStringLiteral("亮灯数量"));
    apply(limits.color_variation_count, QStringLiteral("串色数量"));
    apply(limits.string_light_count, QStringLiteral("串灯数量"));
    apply(limits.mix_color_count, QStringLiteral("混色数量"));
    apply(limits.total_defect_count, QStringLiteral("缺陷总数"));
    return limits;
}

InspectionResult inspectionResultFromOk(bool ok)
{
    return ok ? InspectionResult::Ok : InspectionResult::Ng;
}

common::algorithm::SpecialDetectRule specialDetectRuleFromItem(const SpecialDetectItem &item)
{
    using common::algorithm::HandleMode;
    using common::algorithm::SpecialDetectRule;

    SpecialDetectRule rule;
    rule.enabled              = item.enabled;
    rule.lamp_count_threshold = item.lampThreshold;
    rule.handle_mode          = item.action == QStringLiteral("自动下料")
                                    ? HandleMode::AutoUnload
                                    : HandleMode::Alarm;
    return rule;
}

common::algorithm::SpecialDetectRules specialDetectRulesFromRecipeDocument(const RecipeDocument &doc)
{
    using common::algorithm::SpecialDetectRules;

    const auto itemAt = [&doc](int row) -> SpecialDetectItem {
        if (row >= 0 && row < doc.specialItems.size())
            return doc.specialItems[row];
        return SpecialDetectItem{};
    };

    SpecialDetectRules rules;
    rules.large_area_dead_lamp = specialDetectRuleFromItem(itemAt(0));
    rules.non_edge_defect      = specialDetectRuleFromItem(itemAt(1));
    rules.line_defect          = specialDetectRuleFromItem(itemAt(2));
    rules.block_defect         = specialDetectRuleFromItem(itemAt(3));
    return rules;
}

QString algoThresholdNameAt(int index)
{
    static const QStringList kNames = {
        QStringLiteral("死灯阈值"), QStringLiteral("亮灯阈值"), QStringLiteral("暗灯阈值"),
        QStringLiteral("串灯阈值"), QStringLiteral("串色R"),   QStringLiteral("串色G"),
        QStringLiteral("串色B"),   QStringLiteral("混色初筛"), QStringLiteral("混色筛选"),
    };
    return kNames.value(index);
}

RgbTriple algoRowValuesAt(const AlgoThresholds &algo, int index)
{
    const QString name = algoThresholdNameAt(index);
    for (const AlgoThresholdRow &row : algo.rows) {
        if (row.name == name)
            return row.values;
    }
    return {};
}

float rgbChannelValue(const RgbTriple &rgb, size_t channelIdx)
{
    switch (channelIdx) {
    case 0: return static_cast<float>(rgb.r);
    case 1: return static_cast<float>(rgb.g);
    case 2: return static_cast<float>(rgb.b);
    default: return 0.f;
    }
}

const DetectItemColorSwitches *detectItemSwitchAt(const SystemConfig &cfg, DetectItemKind kind)
{
    for (const DetectItemColorSwitches &item : cfg.detectItems) {
        if (item.item == kind)
            return &item;
    }
    return nullptr;
}

common::algorithm::LightenDetectFeatureFlags featureFlagsFromSystemConfig(const SystemConfig &cfg)
{
    using common::algorithm::LightenDetectFeatureFlags;

    LightenDetectFeatureFlags flags{};

    const auto applyRgb = [&](std::array<bool, common::algorithm::kPureColorChannelCount> &arr,
                              DetectItemKind kind) {
        const DetectItemColorSwitches *item = detectItemSwitchAt(cfg, kind);
        if (!item || !item->masterEnabled) {
            arr.fill(false);
            return;
        }
        arr[0] = item->redEnabled;
        arr[1] = item->greenEnabled;
        arr[2] = item->blueEnabled;
    };

    if (const DetectItemColorSwitches *item = detectItemSwitchAt(cfg, DetectItemKind::BlackScreen))
        flags.black_screen = item->masterEnabled;
    if (const DetectItemColorSwitches *item = detectItemSwitchAt(cfg, DetectItemKind::BlackScreenOn))
        flags.black_light = item->masterEnabled;

    applyRgb(flags.dead_light, DetectItemKind::DeadLed);
    applyRgb(flags.dark_light, DetectItemKind::DimLed);
    applyRgb(flags.bright_light, DetectItemKind::BrightLed);
    applyRgb(flags.color_variation, DetectItemKind::ColorVariation);
    applyRgb(flags.string_light, DetectItemKind::SeriesLed);
    applyRgb(flags.mix_color, DetectItemKind::MixedColor);
    return flags;
}

void applyRoiFromRecipeDebug(common::algorithm::LightenDetectAlgoParams &params,
                             const RecipeDocument &recipe,
                             CameraIndex camera)
{
    const RecipeDebugToolbar *dbg = findDebugEntry(recipe.debugs, camera, ColorChannel::Red);
    if (!dbg || !dbg->roiEnabled || dbg->rois.size() < 8) {
        params.use_auto_roi = true;
        return;
    }

    params.use_auto_roi  = false;
    params.left_top      = {static_cast<int>(dbg->rois[0]), static_cast<int>(dbg->rois[1])};
    params.right_top     = {static_cast<int>(dbg->rois[2]), static_cast<int>(dbg->rois[3])};
    params.right_bottom  = {static_cast<int>(dbg->rois[4]), static_cast<int>(dbg->rois[5])};
    params.left_bottom   = {static_cast<int>(dbg->rois[6]), static_cast<int>(dbg->rois[7])};
    params.image_size    = {recipe.basic.width, recipe.basic.height};
}

void fillPureColorThresholds(common::algorithm::PureColorDetectParams &pure,
                             size_t channelIdx,
                             const AlgoThresholds &algo)
{
    const RgbTriple deadVals   = algoRowValuesAt(algo, 0);
    const RgbTriple brightVals = algoRowValuesAt(algo, 1);
    const RgbTriple dimVals    = algoRowValuesAt(algo, 2);
    const RgbTriple cvVals     = algoRowValuesAt(algo, 4 + static_cast<int>(channelIdx));

    auto &thr = pure.detect_threshold;
    auto &cvt = pure.color_variation_detect_threshold;

    if (channelIdx == 0) {
        thr.thr_dead_R = rgbChannelValue(deadVals, 0);
        thr.thr_dim_max_R = rgbChannelValue(dimVals, 0);
        thr.thr_bright_R = rgbChannelValue(brightVals, 0);
    } else if (channelIdx == 1) {
        thr.thr_dead_G = rgbChannelValue(deadVals, 1);
        thr.thr_dim_max_G = rgbChannelValue(dimVals, 1);
        thr.thr_bright_G = rgbChannelValue(brightVals, 1);
    } else {
        thr.thr_dead_B = rgbChannelValue(deadVals, 2);
        thr.thr_dim_max_B = rgbChannelValue(dimVals, 2);
        thr.thr_bright_B = rgbChannelValue(brightVals, 2);
    }

    cvt.thr_color_variation_red_r   = static_cast<float>(cvVals.r);
    cvt.thr_color_variation_red_g   = static_cast<float>(cvVals.g);
    cvt.thr_color_variation_red_b   = static_cast<float>(cvVals.b);
    cvt.thr_color_variation_green_r = static_cast<float>(cvVals.r);
    cvt.thr_color_variation_green_g = static_cast<float>(cvVals.g);
    cvt.thr_color_variation_green_b = static_cast<float>(cvVals.b);
    cvt.thr_color_variation_blue_r  = static_cast<float>(cvVals.r);
    cvt.thr_color_variation_blue_g  = static_cast<float>(cvVals.g);
    cvt.thr_color_variation_blue_b  = static_cast<float>(cvVals.b);
}

void fillMixColorThresholds(common::algorithm::MixColorVariationDetectParams &mix,
                            size_t channelIdx,
                            const AlgoThresholds &algo)
{
    const RgbTriple preVals = algoRowValuesAt(algo, 7);
    const RgbTriple finVals = algoRowValuesAt(algo, 8);
    auto &mct = mix.mix_color_detect_threshold;

    const float pre = rgbChannelValue(preVals, channelIdx);
    const float fin = rgbChannelValue(finVals, channelIdx);
    if (channelIdx == 0) {
        mct.thr_mix_color_pre_red = pre;
        mct.thr_mix_color_red     = fin;
    } else if (channelIdx == 1) {
        mct.thr_mix_color_pre_green = pre;
        mct.thr_mix_color_green     = fin;
    } else {
        mct.thr_mix_color_pre_blue = pre;
        mct.thr_mix_color_blue     = fin;
    }
}

void fillStringLightThresholds(common::algorithm::StringLightDetectParams &stringParams,
                               size_t channelIdx,
                               const AlgoThresholds &algo)
{
    const RgbTriple vals = algoRowValuesAt(algo, 3);
    const float thr = rgbChannelValue(vals, channelIdx);
    auto &st = stringParams.detect_threshold;
    if (channelIdx == 0) {
        st.thr_string_light_live_r = thr;
        st.thr_string_light_dead_r = thr;
    } else if (channelIdx == 1) {
        st.thr_string_light_live_g = thr;
        st.thr_string_light_dead_g = thr;
    } else {
        st.thr_string_light_live_b = thr;
        st.thr_string_light_dead_b = thr;
    }
}

common::algorithm::LightenDetectAlgoParams lightenDetectAlgoParamsFromRecipe(
    const SystemConfig &cfg,
    const RecipeDocument &recipe,
    int cameraIndex)
{
    using common::algorithm::LightenDetectAlgoParams;
    using common::algorithm::PureDetectColor;

    const PerCameraParams cam = (cameraIndex >= 0 && cameraIndex < recipe.cameras.size())
                                    ? recipe.cameras[cameraIndex]
                                    : PerCameraParams{};
    const int panelW = recipe.basic.width;
    const int panelH = recipe.basic.height;
    const int switchDelayMs = cfg.generalSettings.switchDelayMs;
    const float badPixelThr = static_cast<float>(recipe.algo.badPixelThresh);

    LightenDetectAlgoParams params{};
    params.output_image_root        = cfg.generalSettings.imageStoragePath.toStdString();
    params.output_data_root         = cfg.generalSettings.resultStoragePath.toStdString();
    params.save_original_image_flag = cfg.generalSwitches.saveRawImage;
    params.save_result_image_flag   = cfg.generalSwitches.saveResultImage;

    applyRoiFromRecipeDebug(
        params,
        recipe,
        (cameraIndex >= 0 && cameraIndex < kCameraCount) ? static_cast<CameraIndex>(cameraIndex)
                                                         : CameraIndex::Main);

    params.black_screen.detect_threshold.thr_black_screen =
        static_cast<float>(recipe.algo.blackScreenThresh);
    params.black_screen.led_w = panelW;
    params.black_screen.led_h = panelH;
    params.black_screen.disp_delay_ms = switchDelayMs;
    params.black_screen.black_cam_exposure = cam.blackExposureMs;
    params.black_screen.white_cam_exposure = cam.whiteExposureMs;
    params.black_screen.black_disp_brightness = 0;
    params.black_screen.white_disp_brightness = 255;

    params.black_light.detect_threshold.thr_black_light =
        static_cast<float>(recipe.algo.blackScreenBrightThresh);
    params.black_light.led_w = panelW;
    params.black_light.led_h = panelH;
    params.black_light.cam_exposure = cam.blackExposureMs;
    params.black_light.fixing_defective_thr = badPixelThr;

    params.white.led_w = panelW;
    params.white.led_h = panelH;
    params.white.cam_exposure = cam.whiteExposureMs;
    params.white.fixing_defective_thr = badPixelThr;

    for (size_t ci = 0; ci < common::algorithm::kPureColorChannelCount; ++ci) {
        auto &pure = params.pure_color[ci];
        pure.color = static_cast<PureDetectColor>(ci);
        pure.led_w = panelW;
        pure.led_h = panelH;
        pure.cam_exposure = static_cast<int>(rgbChannelValue(cam.pureExposureMs, ci));
        pure.disp_brightness = static_cast<int>(rgbChannelValue(cam.pureGrayLevel, ci));
        pure.disp_delay_ms = switchDelayMs;
        pure.is_side = cameraIndex != 0;
        pure.angle = cam.rotationDeg;
        pure.fixing_defective_thr = badPixelThr;
        fillPureColorThresholds(pure, ci, recipe.algo);

        auto &mix = params.mix_color[ci];
        mix.color = static_cast<PureDetectColor>(ci);
        mix.led_w = panelW;
        mix.led_h = panelH;
        mix.cam_exposure = static_cast<int>(rgbChannelValue(cam.pureExposureMs, ci));
        mix.fixing_defective_thr = badPixelThr;
        fillMixColorThresholds(mix, ci, recipe.algo);

        auto &stringParams = params.string_light[ci];
        stringParams.color = static_cast<PureDetectColor>(ci);
        stringParams.led_w = panelW;
        stringParams.led_h = panelH;
        stringParams.cam_exposure = static_cast<int>(rgbChannelValue(cam.seriesExposureMs, ci));
        stringParams.disp_brightness = static_cast<int>(rgbChannelValue(cam.seriesGrayLevel, ci));
        stringParams.disp_delay_ms = switchDelayMs;
        stringParams.fixing_defective_thr = badPixelThr;
        fillStringLightThresholds(stringParams, ci, recipe.algo);
    }

    params.applyRoiToSubParams();
    params.applySaveImageFlagsToSubParams();
    return params;
}

QString statusStyle(const QString &s)
{
    if (s == "OK" || s == "PASS" || s == "已完成")
        return QStringLiteral("color:#4caf50;font-weight:bold;font-size:11px;");
    if (s == "NG" || s == "FAIL")
        return QStringLiteral("color:#f44336;font-weight:bold;font-size:11px;");
    if (s == "RW" || s == "进行中")
        return QStringLiteral("color:#ffc107;font-weight:bold;font-size:11px;");
    return QStringLiteral("color:%1;font-size:11px;").arg(kTextMuted);
}

// Pill toggle（工具栏按钮统一等宽等高）
QPushButton *makePill(const QString &text, bool checked, QWidget *parent, int width, int height)
{
    auto *b = new QPushButton(text, parent);
    b->setCheckable(true);
    b->setChecked(checked);
    b->setFixedSize(width, height);
    b->setObjectName(QStringLiteral("homePill"));
    QSizePolicy sp = b->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Fixed);
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    b->setSizePolicy(sp);
    return b;
}

int toolbarPillWidth(int height, const QStringList &extraLabels = {})
{
    static const QStringList fixedLabels = {
        QStringLiteral("红"), QStringLiteral("绿"), QStringLiteral("蓝"),
        QStringLiteral("原图"), QStringLiteral("结果图"),
    };

    QFont pillFont;
    pillFont.setPixelSize(12);
    pillFont.setBold(true);
    const QFontMetrics fm(pillFont);

    int width = 0;
    for (const QString &label : fixedLabels) {
        width = qMax(width, fm.horizontalAdvance(label));
    }
    for (const QString &label : extraLabels) {
        width = qMax(width, fm.horizontalAdvance(label));
    }
    width += 12; // 左右内边距与边框
    return qMax(width, height);
}

void setSnapshotCell(QTableWidget *table, int row, int col, const QString &text, bool isStatus = false)
{
    if (!table || row < 0 || row >= table->rowCount())
        return;

    QTableWidgetItem *item = table->item(row, col);
    if (!item) {
        item = new QTableWidgetItem(text);
        item->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, col, item);
    } else {
        item->setText(text);
    }

    if (!isStatus) {
        item->setData(Qt::ForegroundRole, QBrush(QColor(QStringLiteral("#ffffff"))));
        return;
    }

    const QColor statusColor =
        (text == QStringLiteral("NG")) ? QColor("#f44336") :
        (text == QStringLiteral("OK") || text == QStringLiteral("RW")) ? QColor("#4caf50") :
        QColor("#ffffff");
    item->setData(Qt::ForegroundRole, QBrush(statusColor));
    QFont f = item->font();
    f.setBold(true);
    item->setFont(f);
}

void updateSnapshotTotalCell(QTableWidget *table, int row, DetectItemKind item, int total)
{
    setSnapshotCell(table, row, 4, HomeRuntimeState::formatSnapshotTotal(item, total));
}

void updateSnapshotResultCell(QTableWidget *table, int row, InspectionResult result)
{
    setSnapshotCell(table, row, 5, inspectionResultText(result), true);
}

int snapshotChannelCount(QTableWidget *table, int row, int col)
{
    if (!table || row < 0 || row >= table->rowCount())
        return 0;

    QTableWidgetItem *item = table->item(row, col);
    if (!item || item->text() == QStringLiteral("-"))
        return 0;
    return item->text().toInt();
}

void refreshSnapshotRowAggregate(QTableWidget *table, int row, DetectItemKind item)
{
    const int total = snapshotChannelCount(table, row, 1)
        + snapshotChannelCount(table, row, 2)
        + snapshotChannelCount(table, row, 3);
    updateSnapshotTotalCell(table, row, item, total);
    updateSnapshotResultCell(table, row, inspectionResultFromOk(total == 0));
}

} // namespace

/** @brief HomeView 成员实现（布局构建、主题、工步配置与消息槽） */
namespace lighten {

void HomeView::applyHomeTheme()
{
    setObjectName(QStringLiteral("lightenHomeRoot"));
    setAttribute(Qt::WA_StyledBackground, true);
    // 全局样式仅作用于首页根，避免干扰父级 PageWidget
    setStyleSheet(QStringLiteral(
        "#lightenHomeRoot { background: transparent; }"
        "#lightenHomeRoot QWidget#homeSidebarShell,"
        "#lightenHomeRoot QWidget#homeMainShell {"
        "  background: transparent;"
        "}"

        "#lightenHomeRoot QGroupBox {"
        "  font-weight: 700; font-size: 15px; color: #ffffff;"
        "  background-color: #1E2A3A;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 6px;"
        "  margin-top: 12px;"
        "  padding: 12px 10px 10px 10px;"
        "}"
        "#lightenHomeRoot QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  left: 10px;"
        "  padding: 0 8px;"
        "  background-color: #1E2A3A;"
        "  color: #ffffff;"
        "  font-size: 15px;"
        "  font-weight: 700;"
        "}"
        "#lightenHomeRoot QGroupBox#homeSidebarGroup {"
        "  margin-top: 0px;"
        "  padding-top: 16px;"
        "}"
        "#lightenHomeRoot QGroupBox#homeSidebarGroup::title {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: top left;"
        "  left: 10px;"
        "  top: 0px;"
        "  padding: 0 4px;"
        "  background-color: #1E2A3A;"
        "  color: #ffffff;"
        "  font-size: 15px;"
        "  font-weight: 700;"
        "}"
        "#lightenHomeRoot QGroupBox#homeNoTitleGroup {"
        "  margin-top: 0px;"
        "  padding-top: 10px;"
        "}"
        "#lightenHomeRoot QGroupBox#homeNoTitleGroup::title {"
        "  width: 0px;"
        "  height: 0px;"
        "  padding: 0px;"
        "  margin: 0px;"
        "}"

        "#lightenHomeRoot QFrame#kpiCard {"
        "  background-color: #1E2A3A;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 6px;"
        "}"

        "#lightenHomeRoot QWidget#homeDefectPanel {"
        "  background-color: #0f2239;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 6px;"
        "}"

        "#lightenHomeRoot QWidget#homeImagePreviewRow {"
        "  background: transparent;"
        "  background-color: transparent;"
        "}"

        "#lightenHomeRoot QWidget#homeStationBar {"
        "  background: transparent;"
        "  background-color: transparent;"
        "}"
        "#lightenHomeRoot QFrame#homeStationBlock {"
        "  background-color: #2f3b4c;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 4px;"
        "  min-height: 38px;"
        "}"
        "#lightenHomeRoot QFrame#homeStationBlock[passed=\"true\"] {"
        "  background-color: #1b5e20;"
        "  border: 1px solid #66bb6a;"
        "}"
        "#lightenHomeRoot QFrame#homeStationBlock QLabel#homeStationLabel {"
        "  color: #dbe4ee;"
        "  font-size: 12px;"
        "  background: transparent;"
        "}"
        "#lightenHomeRoot QFrame#homeStationBlock[passed=\"true\"] QLabel#homeStationLabel {"
        "  color: #c8e6c9;"
        "  font-weight: bold;"
        "}"

        "#lightenHomeRoot QFrame#homeImageToolbar {"
        "  background-color: #18263a;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 6px;"
        "  padding: 0px;"
        "  min-height: 40px;"
        "}"
        "#lightenHomeRoot QFrame#homeImageToolbar QPushButton {"
        "  margin-top: 0px;"
        "  margin-bottom: 0px;"
        "}"

        "#lightenHomeRoot QPushButton#homeCtrlBtn {"
        "  min-height: 30px;"
        "  background-color: #184c86;"
        "  color: #e8eef6;"
        "  border: 1px solid #2f82da;"
        "  border-radius: 4px;"
        "  font-weight: bold; font-size: 12px;"
        "}"
        "#lightenHomeRoot QPushButton#homeCtrlBtn:hover { background-color: #1f5ea2; }"
        "#lightenHomeRoot QPushButton#homeCtrlBtn:pressed { background-color: #17477b; }"

        "#lightenHomeRoot QPushButton#homeCtrlBtnDanger {"
        "  min-height: 30px;"
        "  background-color: #5c1e24;"
        "  color: #ffcdd2;"
        "  border: 1px solid #8b2f3a;"
        "  border-radius: 4px;"
        "  font-weight: bold; font-size: 12px;"
        "}"
        "#lightenHomeRoot QPushButton#homeCtrlBtnDanger:hover { background-color: #6e252c; }"

        "#lightenHomeRoot QFrame#homeImageToolbar QPushButton#homePill {"
        "  min-height: 26px; max-height: 26px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 0px; padding-right: 0px;"
        "  margin: 0px;"
        "  background-color: #1e2d40; color: #b8d4f0;"
        "  border: 1px solid #4a6078; border-radius: 7px;"
        "  font-size: 12px;"
        "}"
        "#lightenHomeRoot QWidget#homeDefectPanel QPushButton#homeDefectBtn {"
        "  min-height: 26px; max-height: 26px; min-width: 20px;"
        "  padding-top: 0px; padding-bottom: 0px; padding-left: 4px; padding-right: 4px;"
        "  margin: 0px;"
        "  background-color: #1e2d40; color: #b8d4f0;"
        "  border: 1px solid #4a6078; border-radius: 7px;"
        "  font-size: 12px;"
        "}"
        "#lightenHomeRoot QFrame#homeImageToolbar QPushButton#homePill:checked,"
        "#lightenHomeRoot QWidget#homeDefectPanel QPushButton#homeDefectBtn:checked {"
        "  background-color: #1a3550; color: #ffffff;"
        "  border: 1px solid #42a5f5; font-weight: bold;"
        "}"
        "#lightenHomeRoot QFrame#homeImageToolbar QPushButton#homePill:hover:!checked,"
        "#lightenHomeRoot QWidget#homeDefectPanel QPushButton#homeDefectBtn:hover:!checked {"
        "  background-color: #263a52; border: 1px solid #6b8cae;"
        "}"

        "#lightenHomeRoot QTableWidget#homeResultTable {"
        "  background-color: #1f2d40;"
        "  alternate-background-color: #1f2d40;"
        "  color: #ffffff;"
        "  gridline-color: #3a4a5d;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 4px;"
        "  font-size: 12px;"
        "}"
        "#lightenHomeRoot QTableWidget#homeResultTable::item { padding: 6px 4px; }"
        "#lightenHomeRoot QHeaderView::section {"
        "  background-color: #2f3b4c;"
        "  color: #ffffff;"
        "  padding: 8px 4px;"
        "  border: none;"
        "  border-right: 1px solid #3a4a5d;"
        "  border-bottom: 1px solid #3a4a5d;"
        "  font-weight: bold; font-size: 12px;"
        "}"

        "#lightenHomeRoot QTextEdit#logBox {"
        "  background-color: #161c24;"
        "  color: #dbe4ee;"
        "  border: 1px solid #3a4a5d;"
        "  border-radius: 4px;"
        "  padding: 6px;"
        "  font-family: 'Consolas','Microsoft YaHei UI','Microsoft YaHei',monospace;"
        "  font-size: 11px;"
        "}"
        "#lightenHomeRoot QScrollBar:vertical {"
        "  background: #1a222c; width: 10px; margin: 0;"
        "}"
        "#lightenHomeRoot QScrollBar::handle:vertical {"
        "  background: #3a4a5d; min-height: 24px; border-radius: 4px;"
        "}"
    ));
}

// ── HomeView ─────────────────────────────────────────────────────────────────

HomeView::HomeView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId("lighten.home");
    setPageTitle("首页");
}

HomeView::~HomeView()
{
    detachHardwareEvents();
    MessageCenter::Center::Detach(EVENT_LIGHTEN_IMG_PATH, this, SLOT(slot_img_path(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_FLASH_DETECT_RESULT,
                                  this,
                                  SLOT(slot_flash_detect_result(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_BLACK_SCREEN_DETECT_RESULT,
                                  this,
                                  SLOT(slot_black_screen_detect_result(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_BLACK_SCREEN_LIGHT_DETECT_RESULT,
                                  this,
                                  SLOT(slot_black_screen_light_detect_result(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_LIGHTEN_DEFECT_COUNT_RESULT,
                                  this,
                                  SLOT(slot_lighten_defect_count_result(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_LIGHTEN_DETECT_PROGRESS,
                                  this,
                                  SLOT(slot_lighten_detect_progress(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_LIGHTEN_GENERAL_SETTINGS_CHANGED,
                                  this,
                                  SLOT(slot_general_settings_changed(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_LIGHTEN_SYSTEM_CONFIG_SAVED,
                                  this,
                                  SLOT(slot_system_config_saved(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_LIGHTEN_PRODUCTION_KPI_UPDATED,
                                  this,
                                  SLOT(slot_production_kpi_updated(const QVariantList &)));
}

void HomeView::onLoad()
{
    setUpdatesEnabled(false);
    applyHomeTheme();

    auto *root = new QHBoxLayout(this);
    // 保留外围边距与区域间距，间隙颜色由根背景统一承接
    root->setContentsMargins(6, 6, 6, 6);
    root->setSpacing(8);

    root->addWidget(buildSidebar());
    root->addWidget(buildMainArea(), 1);

    setupViewModel();
    resetDetectProgress();
    resetInspectionSnapshot();

    setUpdatesEnabled(true);
    updateGeometry();

    MessageCenter::Center::Attach(EVENT_LIGHTEN_IMG_PATH, this, SLOT(slot_img_path(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_FLASH_DETECT_RESULT,
                                  this,
                                  SLOT(slot_flash_detect_result(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_BLACK_SCREEN_DETECT_RESULT,
                                  this,
                                  SLOT(slot_black_screen_detect_result(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_BLACK_SCREEN_LIGHT_DETECT_RESULT,
                                  this,
                                  SLOT(slot_black_screen_light_detect_result(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_LIGHTEN_DEFECT_COUNT_RESULT,
                                  this,
                                  SLOT(slot_lighten_defect_count_result(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_LIGHTEN_DETECT_PROGRESS,
                                  this,
                                  SLOT(slot_lighten_detect_progress(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_LIGHTEN_GENERAL_SETTINGS_CHANGED,
                                  this,
                                  SLOT(slot_general_settings_changed(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_LIGHTEN_SYSTEM_CONFIG_SAVED,
                                  this,
                                  SLOT(slot_system_config_saved(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_LIGHTEN_PRODUCTION_KPI_UPDATED,
                                  this,
                                  SLOT(slot_production_kpi_updated(const QVariantList &)));
    attachHardwareEvents();
    refreshHardwareStatus();

    if (m_viewModel)
        m_viewModel->slot_refreshProductionKpiFromService();

    const QString imgPath = SystemConfig::load().generalSettings.imageStoragePath;
    if (!imgPath.isEmpty())
        slot_img_path(QVariantList{imgPath});
}

void HomeView::slot_img_path(const QVariantList &args)
{
    if (args.isEmpty())
        return;

    const QString path = args.at(0).toString();
    if (path.isEmpty() || path == m_img_path)
        return;

    m_img_path = path;
    applyPreviewImage();
}

QString HomeView::previewImagePath() const
{
    const QString baseDir = QDir(m_img_path).filePath(m_camePath);
    const auto filePath = [&baseDir](const char *fileName) {
        return QDir(baseDir).filePath(QString::fromLatin1(fileName));
    };

    if (m_detectItemKind == DetectItemKind::BlackScreenOn) {
        return filePath(m_bOriginal ? "black.jpg" : "black_light.jpg");
    }

    if (m_bOriginal) {
        switch (m_colorChannel) {
        case ColorChannel::Red:
            return filePath("red.jpg");
        case ColorChannel::Green:
            return filePath("green.jpg");
        case ColorChannel::Blue:
            return filePath("blue.jpg");
        default:
            break;
        }
        return QString();
    }

    switch (m_colorChannel) {
    case ColorChannel::Red:
        switch (m_detectItemKind) {
        case DetectItemKind::DeadLed:
            return filePath("red_dead.jpg");
        case DetectItemKind::DimLed:
            return filePath("red_dark.jpg");
        case DetectItemKind::BrightLed:
            return filePath("red_bright.jpg");
        case DetectItemKind::ColorVariation:
            return filePath("red_color.jpg");
        case DetectItemKind::SeriesLed:
            return filePath("red_string_dead.jpg");
        case DetectItemKind::MixedColor:
            return filePath("red_mixcolor.jpg");
        default:
            break;
        }
        break;
    case ColorChannel::Green:
        switch (m_detectItemKind) {
        case DetectItemKind::DeadLed:
            return filePath("green_dead.jpg");
        case DetectItemKind::DimLed:
            return filePath("green_dark.jpg");
        case DetectItemKind::BrightLed:
            return filePath("green_bright.jpg");
        case DetectItemKind::ColorVariation:
            return filePath("green_color.jpg");
        case DetectItemKind::SeriesLed:
            return filePath("green_string_dead.jpg");
        case DetectItemKind::MixedColor:
            return filePath("green_mixcolor.jpg");
        default:
            break;
        }
        break;
    case ColorChannel::Blue:
        switch (m_detectItemKind) {
        case DetectItemKind::DeadLed:
            return filePath("blue_dead.jpg");
        case DetectItemKind::DimLed:
            return filePath("blue_dark.jpg");
        case DetectItemKind::BrightLed:
            return filePath("blue_bright.jpg");
        case DetectItemKind::ColorVariation:
            return filePath("blue_color.jpg");
        case DetectItemKind::SeriesLed:
            return filePath("blue_string_dead.jpg");
        case DetectItemKind::MixedColor:
            return filePath("blue_mixcolor.jpg");
        default:
            break;
        }
        break;
    default:
        break;
    }

    return QString();
}

void HomeView::applyPreviewImage()
{
    if (!m_previewStack)
        return;

    QImage previewImage;
    if (!m_img_path.isEmpty()) {
        const QString path = previewImagePath();
        if (!path.isEmpty())
            previewImage = QImage(path);
    }

    constexpr int kPlaceholderPage = 0;
    constexpr int kGraphicsPage = 1;

    if (!previewImage.isNull() && m_graphicsView && m_previewStack->count() > kGraphicsPage) {
        m_graphicsView->SetImage(0, previewImage);
        m_previewStack->setCurrentIndex(kGraphicsPage);
    } else {
        m_previewStack->setCurrentIndex(kPlaceholderPage);
    }
}

void HomeView::resizeEvent(QResizeEvent *event)
{
    PageWidget::resizeEvent(event);
    applyResultTableWidths();
    applyPreviewImage();
}

void HomeView::applyRuntimeToUi(const HomeRuntimeState &rt)
{
    const QStringList kpiValues = rt.kpiDisplayValues();
    for (int i = 0; i < m_kpiValues.size() && i < kpiValues.size(); ++i) {
        m_kpiValues[i]->setText(kpiValues[i]);
    }

    for (int i = 0; i < m_progressStatus.size() && i < rt.progress.size(); ++i) {
        m_progressStatus[i]->setText(rt.progress[i].statusText);
        m_progressStatus[i]->setStyleSheet(statusStyle(rt.progress[i].statusText));
    }

    if (m_resultInfoLabel) {
        const QString overall = inspectionResultText(rt.snapshot.overallResult);
        const QString color =
            (overall == QStringLiteral("NG")) ? QStringLiteral("#f44336") :
            (overall == QStringLiteral("OK")) ? QStringLiteral("#4caf50") :
                                                QStringLiteral("#ffffff");
        m_resultInfoLabel->setText(
            QStringLiteral("当前SN: <b style='color:#e8eef6;'>%1</b>"
                           " &nbsp;|&nbsp; 总状态: <span style='color:%2;font-weight:bold;'>%3</span>")
                .arg(rt.snapshot.serialNumber.toHtmlEscaped(), color, overall.toHtmlEscaped()));
    }

    if (m_resultTable) {
        for (int r = 0; r < m_resultTable->rowCount() && r < rt.snapshot.rows.size(); ++r) {
            const auto &row = rt.snapshot.rows[r];
            const auto setText = [this, r](int col, const QString &text) {
                setSnapshotCell(m_resultTable, r, col, text);
            };

            setText(0, detectItemName(row.item));
            setText(1, HomeRuntimeState::formatResultCell(row.r));
            setText(2, HomeRuntimeState::formatResultCell(row.g));
            setText(3, HomeRuntimeState::formatResultCell(row.b));
            updateSnapshotTotalCell(m_resultTable, r, row.item, row.total);
            updateSnapshotResultCell(m_resultTable, r, row.result);
        }
    }

    if (m_logEdit) {
        m_logEdit->clear();
        for (const RunLogEntry &entry : rt.logs) {
            appendLog(entry.message, entry.level.isEmpty() ? QStringLiteral("INFO") : entry.level);
        }
    }
}

void HomeView::applyResultTableWidths()
{
    if (!m_resultTable)
        return;

    const int w = m_resultTable->viewport()->width();
    if (w < 240)
        return;

    const int c0 = qMax(90, static_cast<int>(w * 0.24));
    const int c1 = static_cast<int>(w * 0.12);
    const int c2 = static_cast<int>(w * 0.12);
    const int c3 = static_cast<int>(w * 0.12);
    const int c4 = static_cast<int>(w * 0.15);
    const int c5 = w - (c0 + c1 + c2 + c3 + c4);

    m_resultTable->setColumnWidth(0, c0);
    m_resultTable->setColumnWidth(1, c1);
    m_resultTable->setColumnWidth(2, c2);
    m_resultTable->setColumnWidth(3, c3);
    m_resultTable->setColumnWidth(4, c4);
    m_resultTable->setColumnWidth(5, qMax(80, c5));
}

// ── Sidebar ──────────────────────────────────────────────────────────────────

QWidget *HomeView::buildSidebar()
{
    auto *sidebar = new QWidget(this);
    sidebar->setObjectName(QStringLiteral("homeSidebarShell"));
    sidebar->setAttribute(Qt::WA_StyledBackground, true);
    sidebar->setFixedWidth(350);
    auto *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    // ── 设备控制 ──────────────────────────────────────────────────────────
    auto *ctrlBox  = new QGroupBox("设备控制", sidebar);
    ctrlBox->setObjectName(QStringLiteral("homeSidebarGroup"));
    auto *ctrlGrid = new QGridLayout(ctrlBox);
    ctrlGrid->setHorizontalSpacing(8);
    ctrlGrid->setVerticalSpacing(8);
    ctrlGrid->setContentsMargins(10, 18, 10, 6);
    ctrlGrid->setAlignment(Qt::AlignTop);

    struct CtrlBtn { QString label; bool danger; };
    const QVector<CtrlBtn> ctrls = {
        {"初始化", false}, {"启动",     false},
        {"停止",   true},  {"复位",     false},
        {"继续",   false}, {"清除报警", false},
        {"清料",   false}, {"单次调试", false},
        {"统计数据清零", false},
    };
    for (int i = 0; i < ctrls.size(); ++i) {
        auto *btn = new QPushButton(ctrls[i].label, ctrlBox);
        btn->setObjectName(ctrls[i].danger ? QStringLiteral("homeCtrlBtnDanger")
                                           : QStringLiteral("homeCtrlBtn"));
        btn->setFixedHeight(40);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        if (ctrls[i].label == QStringLiteral("初始化")) {
            connect(btn, &QPushButton::clicked, this, &HomeView::slot_initialize);
        } else if (ctrls[i].label == QStringLiteral("统计数据清零")) {
            connect(btn, &QPushButton::clicked, this, &HomeView::clearProductionKpi);
        }
        ctrlGrid->addWidget(btn, i / 2, i % 2);
    }
    layout->addWidget(ctrlBox, 5);

    // ── 硬件连接状态 ──────────────────────────────────────────────────────
    m_hwBox = new QGroupBox("硬件连接状态", sidebar);
    m_hwBox->setObjectName(QStringLiteral("homeSidebarGroup"));
    m_hwLayout = new QVBoxLayout(m_hwBox);
    m_hwLayout->setSpacing(1);
    m_hwLayout->setContentsMargins(10, 18, 10, 6);
    m_hwLayout->setAlignment(Qt::AlignTop);
    buildHardwareStatusRows();
    layout->addWidget(m_hwBox, 6);
    return sidebar;
}

void HomeView::buildHardwareStatusRows()
{
    if (!m_hwLayout)
        return;

    m_hwRows.clear();
    m_motionCardNames.clear();
    while (QLayoutItem *item = m_hwLayout->takeAt(0)) {
        if (QWidget *widget = item->widget())
            widget->deleteLater();
        delete item;
    }

    const auto addRow = [this](const QString &displayName,
                               HwDeviceKind kind,
                               const QString &deviceKey) {
        auto *row = new QWidget(m_hwBox);
        row->setFixedHeight(24);
        row->setAttribute(Qt::WA_StyledBackground, true);
        row->setStyleSheet(QStringLiteral("background-color:transparent; border:none;"));
        auto *rl = new QHBoxLayout(row);
        rl->setContentsMargins(0, 0, 0, 0);
        rl->setSpacing(6);

        auto *dot = new QLabel(QStringLiteral("●"), row);
        dot->setFixedWidth(12);

        auto *name = new QLabel(displayName, row);
        name->setStyleSheet(QStringLiteral("color:#e8eef6; font-size:14px;"));
        auto *status = new QLabel(row);
        status->setStyleSheet(QStringLiteral("color:#9aa4b5; font-size:13px;"));

        rl->addWidget(dot);
        rl->addWidget(name, 1);
        rl->addWidget(status);
        m_hwLayout->addWidget(row);

        auto *sep = new QFrame(m_hwBox);
        sep->setFrameShape(QFrame::HLine);
        sep->setFixedHeight(1);
        sep->setStyleSheet(QStringLiteral(
            "background:transparent;border:none;border-top:1px dashed #3a4a5d;"));
        m_hwLayout->addWidget(sep);

        HwStatusRow hwRow;
        hwRow.kind        = kind;
        hwRow.displayName = displayName;
        hwRow.deviceKey   = deviceKey;
        hwRow.dot         = dot;
        hwRow.status      = status;
        m_hwRows.append(hwRow);
    };

    for (const std::string &key : DeviceConfigReader::loadCameraKeys()) {
        addRow(QString::fromStdString(key), HwDeviceKind::Camera, QString::fromStdString(key));
    }

    for (const std::string &cardName : DeviceConfigReader::loadMotionCardNames())
        m_motionCardNames.append(QString::fromStdString(cardName));
    addRow(QStringLiteral("运控模块"), HwDeviceKind::Motion, {});

    const std::vector<std::string> scannerKeys = DeviceConfigReader::loadScannerKeys();
    addRow(QStringLiteral("扫码枪"),
           HwDeviceKind::Scanner,
           scannerKeys.empty() ? QString() : QString::fromStdString(scannerKeys.front()));
}

void HomeView::refreshHardwareStatus()
{
    const auto dm = tryGetDeviceManager();

    const auto setRowState = [](HwStatusRow &row, bool active) {
        const QString statusText = active ? QStringLiteral("已连接") : QStringLiteral("已断开");
        const QString color = active ? QStringLiteral("#4caf50") : QStringLiteral("#f44336");
        row.dot->setStyleSheet(QStringLiteral("color:%1; font-size:13px;").arg(color));
        row.status->setText(statusText);
    };

    for (HwStatusRow &row : m_hwRows) {
        bool active = false;
        switch (row.kind) {
        case HwDeviceKind::Camera: {
            if (dm && !row.deviceKey.isEmpty()) {
                const auto camera = dm->CameraHandler(row.deviceKey.toStdString());
                active = camera && camera->IsConnected();
            }
            break;
        }
        case HwDeviceKind::Motion: {
            if (dm && !m_motionCardNames.isEmpty()) {
                const auto motion = dm->MotionHandler();
                active = motion != nullptr;
                if (active) {
                    for (const QString &cardName : m_motionCardNames) {
                        if (!motion->GetCardConnectedState(cardName.toStdString())) {
                            active = false;
                            break;
                        }
                    }
                }
            }
            break;
        }
        case HwDeviceKind::Scanner: {
            if (dm && !row.deviceKey.isEmpty()) {
                const auto scanner = dm->ScannerHandler(row.deviceKey.toStdString());
                active = scanner && scanner->IsConnected();
            }
            break;
        }
        }
        setRowState(row, active);
        appendHardwareConnLog(row.displayName, active);
    }
}

void HomeView::appendHardwareConnLog(const QString &deviceName, bool connected)
{
    if (deviceName.isEmpty())
        return;

    if (!m_viewModel)
        setupViewModel();

    const QString message = connected
                                ? QStringLiteral("%1 连接成功").arg(deviceName)
                                : QStringLiteral("%1 连接失败").arg(deviceName);
    const QString level = connected ? QStringLiteral("INFO") : QStringLiteral("WARN");
    m_viewModel->slot_appendLog(message, level);
}

void HomeView::attachHardwareEvents()
{
    MessageCenter::Center::Attach(EVENT_CAMERA_CONN_STATE,
                                  this,
                                  SLOT(slot_camera_conn_state(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_MOTION_CONN_STATE,
                                  this,
                                  SLOT(slot_motion_conn_state(const QVariantList &)));
    MessageCenter::Center::Attach(EVENT_SCANNER_CONN_STATE,
                                  this,
                                  SLOT(slot_scanner_conn_state(const QVariantList &)));
}

void HomeView::detachHardwareEvents()
{
    MessageCenter::Center::Detach(EVENT_CAMERA_CONN_STATE,
                                  this,
                                  SLOT(slot_camera_conn_state(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_MOTION_CONN_STATE,
                                  this,
                                  SLOT(slot_motion_conn_state(const QVariantList &)));
    MessageCenter::Center::Detach(EVENT_SCANNER_CONN_STATE,
                                  this,
                                  SLOT(slot_scanner_conn_state(const QVariantList &)));
}

void HomeView::slot_camera_conn_state(const QVariantList &args)
{
    if (args.size() < 2)
        return;

    const QString cameraName = args.at(0).toString();
    const bool connected = args.at(1).toBool();
    for (HwStatusRow &row : m_hwRows) {
        if (row.kind != HwDeviceKind::Camera || row.deviceKey != cameraName)
            continue;

        const QString statusText =
            connected ? QStringLiteral("已连接") : QStringLiteral("已断开");
        const QString color = connected ? QStringLiteral("#4caf50") : QStringLiteral("#f44336");
        row.dot->setStyleSheet(QStringLiteral("color:%1; font-size:13px;").arg(color));
        row.status->setText(statusText);
        appendHardwareConnLog(cameraName, connected);
        break;
    }
}

void HomeView::slot_flash_detect_result(const QVariantList &args)
{
    if (args.isEmpty())
        return;

    const int row = static_cast<int>(DetectItemKind::Flash);
    updateSnapshotResultCell(m_resultTable, row, inspectionResultFromOk(args.at(0).toBool()));
}

void HomeView::slot_black_screen_detect_result(const QVariantList &args)
{
    if (args.isEmpty())
        return;

    const int row = static_cast<int>(DetectItemKind::BlackScreen);
    updateSnapshotResultCell(m_resultTable, row, inspectionResultFromOk(args.at(0).toBool()));
}

void HomeView::slot_black_screen_light_detect_result(const QVariantList &args)
{
    if (args.isEmpty())
        return;

    const int count = args.at(0).toInt();
    const int row = static_cast<int>(DetectItemKind::BlackScreenOn);
    updateSnapshotTotalCell(m_resultTable, row, DetectItemKind::BlackScreenOn, count);
    updateSnapshotResultCell(m_resultTable, row, inspectionResultFromOk(count == 0));
}

void HomeView::slot_lighten_defect_count_result(const QVariantList &args)
{
    if (args.size() < 3)
        return;

    const int defect_kind = args.at(0).toInt();
    const int color_idx = args.at(1).toInt();
    const int count = args.at(2).toInt();
    if (defect_kind < 0
        || defect_kind >= static_cast<int>(DetectItemKind::BlackScreen)
        || color_idx < 0
        || color_idx > 2) {
        return;
    }

    const int row = defect_kind;
    const int col = 1 + color_idx;
    setSnapshotCell(m_resultTable, row, col, HomeRuntimeState::formatResultCell(count));
    refreshSnapshotRowAggregate(m_resultTable, row, static_cast<DetectItemKind>(defect_kind));
}

void HomeView::slot_lighten_detect_progress(const QVariantList &args)
{
    if (args.size() < 3)
        return;

    const int itemKind  = args.at(0).toInt();
    const int colorIdx  = args.at(1).toInt();
    const int statusInt = args.at(2).toInt();
    if (itemKind < 0 || itemKind >= kDetectItemCount)
        return;

    if (!m_viewModel)
        setupViewModel();

    const SystemConfig             cfg  = SystemConfigRepository::load();
    const DetectItemKind           kind = static_cast<DetectItemKind>(itemKind);
    const DetectItemColorSwitches *item = detectItemSwitchAt(cfg, kind);

    if (detectItemHasRgbProgress(kind)) {
        if (colorIdx < 0 || colorIdx > 2)
            return;
        if (item && !item->showsHomeProgressSlot(colorIdx))
            return;
    } else {
        if (colorIdx != common::algorithm::kLightenDetectProgressColorNone)
            return;
        if (item && !item->masterEnabled)
            return;
    }

    const int slot = detectItemHasRgbProgress(kind) ? homeProgressSlotIndex(kind, colorIdx)
                                                    : homeProgressSlotIndex(kind);
    if (slot < 0)
        return;

    const QString statusText = detectProgressStatusText(statusInt);

    HomeRuntimeState rt = m_viewModel->runtime();
    if (rt.progress.size() != kHomeProgressCount)
        rt.progress.resize(kHomeProgressCount);
    if (slot >= m_progressStatus.size() || slot >= rt.progress.size())
        return;
    if (rt.progress[slot].statusText == statusText)
        return;

    rt.progress[slot].statusText = statusText;
    m_progressStatus[slot]->setText(statusText);
    m_progressStatus[slot]->setStyleSheet(statusStyle(statusText));
    m_viewModel->setRuntime(rt);
}

void HomeView::slot_motion_conn_state(const QVariantList &args)
{
    Q_UNUSED(args);
    for (HwStatusRow &row : m_hwRows) {
        if (row.kind != HwDeviceKind::Motion)
            continue;

        const auto dm = tryGetDeviceManager();
        bool connected = false;
        if (dm && !m_motionCardNames.isEmpty()) {
            const auto motion = dm->MotionHandler();
            connected = motion != nullptr;
            if (connected) {
                for (const QString &cardName : m_motionCardNames) {
                    if (!motion->GetCardConnectedState(cardName.toStdString())) {
                        connected = false;
                        break;
                    }
                }
            }
        }

        const QString statusText =
            connected ? QStringLiteral("已连接") : QStringLiteral("已断开");
        const QString color = connected ? QStringLiteral("#4caf50") : QStringLiteral("#f44336");
        row.dot->setStyleSheet(QStringLiteral("color:%1; font-size:13px;").arg(color));
        row.status->setText(statusText);
        appendHardwareConnLog(row.displayName, connected);
        break;
    }
}

void HomeView::slot_scanner_conn_state(const QVariantList &args)
{
    if (args.size() < 2)
        return;

    const QString scannerName = args.at(0).toString();
    const bool connected = args.at(1).toBool();
    for (HwStatusRow &row : m_hwRows) {
        if (row.kind != HwDeviceKind::Scanner || row.deviceKey != scannerName)
            continue;

        const QString statusText =
            connected ? QStringLiteral("已连接") : QStringLiteral("已断开");
        const QString color = connected ? QStringLiteral("#4caf50") : QStringLiteral("#f44336");
        row.dot->setStyleSheet(QStringLiteral("color:%1; font-size:13px;").arg(color));
        row.status->setText(statusText);
        appendHardwareConnLog(row.displayName, connected);
        break;
    }
}

void HomeView::slot_general_settings_changed(const QVariantList &args)
{
    if (!m_viewModel)
        setupViewModel();

    int maxUseCount = 100000;
    if (args.size() >= 2) {
        maxUseCount = args.at(1).toInt();
    } else {
        maxUseCount = SystemConfigRepository::load().generalSettings.maxUseCount;
    }
    m_viewModel->slot_updateMaxUseCount(maxUseCount);
}

void HomeView::slot_production_kpi_updated(const QVariantList &args)
{
    Q_UNUSED(args);
    if (!m_viewModel)
        setupViewModel();
    m_viewModel->slot_refreshProductionKpiFromService();
}

void HomeView::clearProductionKpi()
{
    auto *service = PluginManager::getInterface<ILightenProductionKpiService>();
    if (!service || !service->isReady()) {
        appendLog(QStringLiteral("生产 KPI 服务未就绪，无法清空"), QStringLiteral("WARN"));
        return;
    }

    QString error;
    if (!service->upsertKpi(LightenProductionKpiRecord{}, &error)) {
        appendLog(error.isEmpty() ? QStringLiteral("清空生产 KPI 失败")
                                  : QStringLiteral("清空生产 KPI 失败: %1").arg(error),
                  QStringLiteral("ERROR"));
        return;
    }

    MessageCenter::Center::Notify(EVENT_LIGHTEN_PRODUCTION_KPI_UPDATED, QVariantList{});
}

void HomeView::slot_system_config_saved(const QVariantList &args)
{
    Q_UNUSED(args);
    refreshDetectProgressLayout();
}

void HomeView::slot_initialize()
{
    resetDetectProgress();
    resetInspectionSnapshot();
    buildStepConfigFromSystemConfig();
}

// ── Main area wrapper ────────────────────────────────────────────────────────

QWidget *HomeView::buildMainArea()
{
    auto *main   = new QWidget(this);
    main->setObjectName(QStringLiteral("homeMainShell"));
    main->setAttribute(Qt::WA_StyledBackground, true);
    auto *layout = new QVBoxLayout(main);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    buildKpiBar(layout);
    buildContentArea(layout);
    return main;
}

// ── KPI Bar ──────────────────────────────────────────────────────────────────

void HomeView::buildKpiBar(QVBoxLayout *root)
{
    const HomeRuntimeState demo = HomeRuntimeState::defaultDemo();
    const QStringList labels = HomeRuntimeState::kpiLabels();
    const QStringList values = demo.kpiDisplayValues();

    auto *bar    = new QWidget(this);
    bar->setMinimumHeight(58);
    auto *layout = new QHBoxLayout(bar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    for (int i = 0; i < labels.size(); ++i) {
        auto *card = new QFrame(bar);
        card->setObjectName(QStringLiteral("kpiCard"));
        card->setFrameShape(QFrame::StyledPanel);
        card->setMinimumHeight(58);

        auto *vl = new QVBoxLayout(card);
        vl->setContentsMargins(10, 8, 10, 8);
        vl->setSpacing(3);

        auto *lbl = new QLabel(labels[i], card);
        lbl->setStyleSheet(QStringLiteral("color:#9aa4b5; font-size:12px; background:transparent;"));

        auto *val = new QLabel(i < values.size() ? values[i] : QString(), card);
        val->setStyleSheet(QStringLiteral("color:#e8eef6; background:transparent;"));
        QFont f = val->font();
        f.setPointSize(15);
        f.setBold(true);
        val->setFont(f);

        vl->addWidget(lbl);
        vl->addWidget(val);
        layout->addWidget(card, 1);
        m_kpiValues.append(val);
    }
    root->addWidget(bar);
}

// ── Content area ─────────────────────────────────────────────────────────────

void HomeView::buildContentArea(QVBoxLayout *root)
{
    auto *area   = new QWidget(this);
    auto *layout = new QHBoxLayout(area);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    // 左列：图像面板 + 运行日志
    auto *leftCol  = new QWidget(area);
    auto *leftVbox = new QVBoxLayout(leftCol);
    leftVbox->setContentsMargins(0, 0, 0, 0);
    leftVbox->setSpacing(6);
    leftVbox->addWidget(buildImagePanel(), 1);
    leftVbox->addWidget(buildLogCard());

    layout->addWidget(leftCol, 14);
    layout->addWidget(buildRightStack(), 6);

    root->addWidget(area, 1);
}

// ── Image panel (left) ────────────────────────────────────────────────────────

QWidget *HomeView::buildImagePanel()
{
    auto *box    = new QGroupBox(QString(), this);
    box->setObjectName(QStringLiteral("homeNoTitleGroup"));
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    // ── Toolbar row ────────────────────────────────────────────────────────
    // 工具条：与配方调试区一致——单行 HBox + 上下边距，勿把按钮行锁死在 26px 容器内（QSS 1px 边框会裁底）
    constexpr int kHomeToolbarCtrlH = 26;
    constexpr int kHomeToolbarGroupInnerSpacing = 5;
    constexpr int kHomeToolbarGroupOuterSpacing = 20;

    auto *toolbar = new QFrame(box);
    toolbar->setObjectName(QStringLiteral("homeImageToolbar"));
    toolbar->setFrameShape(QFrame::NoFrame);
    toolbar->setAttribute(Qt::WA_StyledBackground, true);
    toolbar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    auto *tlLayout = new QHBoxLayout(toolbar);
    tlLayout->setContentsMargins(8, 6, 8, 6);
    tlLayout->setSpacing(kHomeToolbarGroupOuterSpacing);
    tlLayout->setAlignment(Qt::AlignVCenter);

    m_cameraMap = DeviceConfigReader::loadCameraMap();
    QStringList cameraKeys;
    cameraKeys.reserve(static_cast<int>(m_cameraMap.size()));
    for (const auto &entry : m_cameraMap) {
        cameraKeys.append(QString::fromStdString(entry.first));
    }
    const int pillW = toolbarPillWidth(kHomeToolbarCtrlH, cameraKeys);

    auto addPillGroup = [&](QButtonGroup *group,
                              std::initializer_list<std::pair<const char *, bool>> items) {
        auto *groupWrap = new QWidget(toolbar);
        groupWrap->setAttribute(Qt::WA_StyledBackground, true);
        auto *groupLayout = new QHBoxLayout(groupWrap);
        groupLayout->setContentsMargins(0, 0, 0, 0);
        groupLayout->setSpacing(kHomeToolbarGroupInnerSpacing);
        groupLayout->setAlignment(Qt::AlignVCenter);
        int pillIndex = 0;
        for (auto [text, on] : items) {
            auto *b = makePill(text, on, groupWrap, pillW, kHomeToolbarCtrlH);
            group->addButton(b, pillIndex++);
            groupLayout->addWidget(b, 0, Qt::AlignVCenter);
        }
        tlLayout->addWidget(groupWrap, 0, Qt::AlignVCenter);
    };

    // Color group
    auto *colorGroup = new QButtonGroup(toolbar);
    colorGroup->setExclusive(true);
    addPillGroup(colorGroup, {{"红", true}, {"绿", false}, {"蓝", false}});
    connect(colorGroup, &QButtonGroup::idClicked, this, [this, colorGroup](int id) {
        const auto *btn = qobject_cast<QPushButton *>(colorGroup->button(id));
        if (!btn)
            return;
        if (btn->text() == QStringLiteral("红"))
            m_colorChannel = ColorChannel::Red;
        else if (btn->text() == QStringLiteral("绿"))
            m_colorChannel = ColorChannel::Green;
        else if (btn->text() == QStringLiteral("蓝"))
            m_colorChannel = ColorChannel::Blue;
        applyPreviewImage();
    });

    // Camera group（按钮名称与数量由 config/device_config.json 中 DeviceManager.Camera 决定）
    if (!m_cameraMap.empty()) {
        auto *camGroup = new QButtonGroup(toolbar);
        camGroup->setExclusive(true);
        auto *camWrap = new QWidget(toolbar);
        camWrap->setAttribute(Qt::WA_StyledBackground, true);
        auto *camLayout = new QHBoxLayout(camWrap);
        camLayout->setContentsMargins(0, 0, 0, 0);
        camLayout->setSpacing(kHomeToolbarGroupInnerSpacing);
        camLayout->setAlignment(Qt::AlignVCenter);
        for (int i = 0; i < cameraKeys.size(); ++i) {
            auto *b = makePill(cameraKeys.at(i), i == 0, camWrap, pillW, kHomeToolbarCtrlH);
            camGroup->addButton(b, i);
            camLayout->addWidget(b, 0, Qt::AlignVCenter);
        }
        connect(camGroup, &QButtonGroup::idClicked, this, [this, camGroup](int id) {
            const auto *btn = qobject_cast<QPushButton *>(camGroup->button(id));
            if (!btn)
                return;
            const auto it = m_cameraMap.find(btn->text().toStdString());
            if (it != m_cameraMap.end())
                m_camePath = QString::fromStdString(it->second);
            applyPreviewImage();
        });
        if (!cameraKeys.isEmpty()) {
            const auto it = m_cameraMap.find(cameraKeys.at(0).toStdString());
            if (it != m_cameraMap.end())
                m_camePath = QString::fromStdString(it->second);
        }
        tlLayout->addWidget(camWrap, 0, Qt::AlignVCenter);
    }

    // View toggle (source / result)
    auto *viewGroup = new QButtonGroup(toolbar);
    viewGroup->setExclusive(true);
    addPillGroup(viewGroup, {{"原图", true}, {"结果图", false}});
    connect(viewGroup, &QButtonGroup::idClicked, this, [this, viewGroup](int id) {
        const auto *btn = qobject_cast<QPushButton *>(viewGroup->button(id));
        if (!btn)
            return;
        m_bOriginal = (btn->text() == QStringLiteral("原图"));
        applyPreviewImage();
    });

    tlLayout->addStretch();

    layout->addWidget(toolbar);
    layout->addSpacing(2);

    // ── Image area: defect pills overlay + preview frame ──────────────────
    auto *imageRow    = new QWidget(box);
    imageRow->setObjectName(QStringLiteral("homeImagePreviewRow"));
    imageRow->setAttribute(Qt::WA_StyledBackground, true);
    auto *imageLayout = new QHBoxLayout(imageRow);
    imageLayout->setContentsMargins(0, 0, 0, 0);
    imageLayout->setSpacing(10);

    // Left: defect type pills (vertical, exclusive)
    auto *defectPanel  = new QWidget(imageRow);
    defectPanel->setObjectName(QStringLiteral("homeDefectPanel"));
    defectPanel->setAttribute(Qt::WA_StyledBackground, true);
    defectPanel->setFixedWidth(100);
    auto *defectLayout = new QVBoxLayout(defectPanel);
    defectLayout->setContentsMargins(6, 6, 6, 6);
    defectLayout->setSpacing(6);
    auto *defectGroup = new QButtonGroup(defectPanel);
    defectGroup->setExclusive(true);
    for (int i = 0; i < kDefectTypes.size(); ++i) {
        auto *b = new QPushButton(kDefectTypes[i], defectPanel);
        b->setCheckable(true);
        b->setChecked(i == 0);
        b->setObjectName(QStringLiteral("homeDefectBtn"));
        b->setAttribute(Qt::WA_StyledBackground, true);
        b->setFixedHeight(26);
        defectGroup->addButton(b, i);
        defectLayout->addWidget(b);
    }
    connect(defectGroup, &QButtonGroup::idClicked, this, [this](int id) {
        if (id >= 0 && id < kDefectTypes.size())
            m_detectItemKind = static_cast<DetectItemKind>(id);
        applyPreviewImage();
    });
    defectLayout->addStretch();
    imageLayout->addWidget(defectPanel);

    // Right: preview pane (placeholder + optional graphics view in stack)
    m_previewStack = new QStackedWidget(imageRow);
    auto *placeholderPane = new QWidget(m_previewStack);
    placeholderPane->setAttribute(Qt::WA_StyledBackground, true);
    placeholderPane->setStyleSheet(QStringLiteral("background-color: #000000;"));
    m_previewStack->addWidget(placeholderPane);

    m_graphicsView = PluginManager::getInterface<IGraphicsVew>();
    if (m_graphicsView)
    {
        QWidget *graphicsPane = m_graphicsView->CreateWidget(0);
        m_graphicsView->SetVisible(0, true, false);
        m_previewStack->addWidget(graphicsPane);
    }

    m_previewStack->setCurrentWidget(placeholderPane);
    m_previewStack->setMinimumHeight(200);
    imageLayout->addWidget(m_previewStack, 1);
    layout->addWidget(imageRow, 1);

    // ── Station track（进料 / 检测 / 出料，passed 为通过态显示绿色）──────────
    auto *stationBar = new QWidget(box);
    stationBar->setObjectName(QStringLiteral("homeStationBar"));
    stationBar->setAttribute(Qt::WA_StyledBackground, true);
    auto *stationLayout = new QHBoxLayout(stationBar);
    stationLayout->setContentsMargins(0, 4, 0, 0);
    stationLayout->setSpacing(10);

    auto makeStation = [&](const QString &name, bool passed) {
        auto *blk = new QFrame(stationBar);
        blk->setObjectName(QStringLiteral("homeStationBlock"));
        blk->setProperty("passed", passed);
        blk->setAttribute(Qt::WA_StyledBackground, true);
        blk->setMinimumHeight(38);
        blk->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto *hl = new QHBoxLayout(blk);
        hl->setContentsMargins(12, 8, 12, 8);
        auto *lbl = new QLabel(name, blk);
        lbl->setObjectName(QStringLiteral("homeStationLabel"));
        lbl->setAlignment(Qt::AlignCenter);
        hl->addWidget(lbl);
        return blk;
    };

    auto makeArrow = [&]() {
        auto *a = new QLabel(QStringLiteral("→"), stationBar);
        a->setStyleSheet(QStringLiteral("color:#5c6b82; font-size:16px; font-weight:bold; background:transparent;"));
        a->setAlignment(Qt::AlignCenter);
        return a;
    };

    stationLayout->addWidget(makeStation(QStringLiteral("进料"), true), 1);
    stationLayout->addWidget(makeArrow());
    stationLayout->addWidget(makeStation(QStringLiteral("检测"), false), 1);
    stationLayout->addWidget(makeArrow());
    stationLayout->addWidget(makeStation(QStringLiteral("出料"), false), 1);
    layout->addWidget(stationBar);

    return box;
}

// ── Right stack (progress + result) ─────────────────────────────────────────

QWidget *HomeView::buildRightStack()
{
    auto *panel  = new QWidget(this);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(buildProgressCard(), 5);
    layout->addWidget(buildResultCard(), 7);
    return panel;
}

QWidget *HomeView::buildProgressCard()
{
    const HomeRuntimeState demo = HomeRuntimeState::defaultDemo();

    auto *box  = new QGroupBox("检测项进度", this);
    box->setObjectName(QStringLiteral("homeSidebarGroup"));
    box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    auto *grid = new QGridLayout(box);
    m_progressGrid = grid;
    grid->setHorizontalSpacing(kProgressGridHSpacing);
    grid->setVerticalSpacing(kProgressGridVSpacing);
    grid->setContentsMargins(12, kProgressTitleGap, 12, 10);
    grid->setAlignment(Qt::AlignTop);

    m_progressCells.clear();
    m_progressStatus.clear();

    for (int slot = 0; slot < kHomeProgressCount; ++slot) {
        auto *cell = new QWidget(box);
        cell->setAttribute(Qt::WA_StyledBackground, true);
        cell->setStyleSheet(QStringLiteral("background-color:transparent; border:none;"));
        auto *cl = new QHBoxLayout(cell);
        cl->setContentsMargins(0, 0, 0, 0);
        cl->setSpacing(kProgressInnerSpacing);

        auto *name = new QLabel(homeProgressSlotLabel(slot), cell);
        name->setStyleSheet(QStringLiteral("font-size:12px; color:#dbe4ee;"));

        const QString statusText =
            slot < demo.progress.size() ? demo.progress[slot].statusText : QString();
        auto *status = new QLabel(statusText, cell);
        status->setFixedWidth(kProgressStatusW);
        status->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        status->setStyleSheet(statusStyle(statusText));

        cl->addWidget(name);
        cl->addStretch(1);
        cl->addWidget(status);

        m_progressCells.append(cell);
        m_progressStatus.append(status);
    }

    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    refreshDetectProgressLayout();
    return box;
}

void HomeView::refreshDetectProgressLayout()
{
    if (!m_progressGrid || m_progressCells.isEmpty())
        return;

    const SystemConfig cfg = SystemConfigRepository::load();

    int visibleSlot = 0;
    for (int slot = 0; slot < m_progressCells.size(); ++slot) {
        QWidget *cell = m_progressCells[slot];
        DetectItemKind kind = DetectItemKind::DeadLed;
        int            colorIdx = -1;
        decodeHomeProgressSlot(slot, &kind, &colorIdx);

        const DetectItemColorSwitches *item = detectItemSwitchAt(cfg, kind);
        const bool visible = item
                                 ? (colorIdx >= 0 ? item->showsHomeProgressSlot(colorIdx)
                                                  : item->masterEnabled)
                                 : true;

        cell->setVisible(visible);
        m_progressGrid->removeWidget(cell);

        if (visible) {
            const int row = visibleSlot / 2;
            m_progressGrid->addWidget(cell, row, visibleSlot % 2, Qt::AlignTop);
            ++visibleSlot;
        }
    }

    const int visibleRows = visibleSlot > 0 ? (visibleSlot + 1) / 2 : 0;
    const int maxRows     = (kHomeProgressCount + 1) / 2;
    for (int r = 0; r <= maxRows; ++r)
        m_progressGrid->setRowStretch(r, r == visibleRows ? 1 : 0);
}

QWidget *HomeView::buildResultCard()
{
    auto *box    = new QGroupBox("当前检测结果快照", this);
    box->setObjectName(QStringLiteral("homeSidebarGroup"));
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(10, 18, 10, 8);
    layout->setSpacing(6);

    m_resultInfoLabel = new QLabel(box);
    m_resultInfoLabel->setTextFormat(Qt::RichText);
    m_resultInfoLabel->setStyleSheet(QStringLiteral("color:#b9c6d4; font-size:13px; margin-bottom:2px;"));
    layout->addWidget(m_resultInfoLabel);

    m_resultTable = new QTableWidget(kDetectItemCount, 6, box);
    m_resultTable->setObjectName(QStringLiteral("homeResultTable"));
    m_resultTable->setHorizontalHeaderLabels({"检测项", "R", "G", "B", "总计", "结果"});
    auto *hh = m_resultTable->horizontalHeader();
    hh->setSectionsMovable(false);
    hh->setStretchLastSection(false);
    hh->setSectionResizeMode(QHeaderView::Fixed);
    m_resultTable->verticalHeader()->setVisible(false);
    m_resultTable->verticalHeader()->setDefaultSectionSize(38);
    m_resultTable->setShowGrid(true);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultTable->setAlternatingRowColors(false);
    m_resultTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    layout->addWidget(m_resultTable, 1);
    applyResultTableWidths();
    return box;
}

// ── Log card ─────────────────────────────────────────────────────────────────

QWidget *HomeView::buildLogCard()
{
    auto *box    = new QGroupBox("运行日志", this);
    box->setObjectName(QStringLiteral("homeSidebarGroup"));
    box->setFixedHeight(148);
    auto *layout = new QVBoxLayout(box);
    layout->setContentsMargins(10, 16, 10, 10);

    m_logEdit = new QTextEdit(box);
    m_logEdit->setReadOnly(true);
    m_logEdit->setObjectName(QStringLiteral("logBox"));
    m_logEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_logEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_logEdit, &QTextEdit::customContextMenuRequested, this, [this](const QPoint &pos) {
        if (!m_logEdit)
            return;
        QMenu menu(m_logEdit);
        QAction *clearAct = menu.addAction(QStringLiteral("清屏"));
        if (menu.exec(m_logEdit->mapToGlobal(pos)) == clearAct)
            m_logEdit->clear();
    });
    layout->addWidget(m_logEdit);

    return box;
}

// ── Public update API ────────────────────────────────────────────────────────

void HomeView::updateKpi(int index, const QString &value)
{
    if (index >= 0 && index < m_kpiValues.size())
        m_kpiValues[index]->setText(value);
}

void HomeView::updateProgressStatus(int slotIndex, const QString &statusText)
{
    if (slotIndex < 0 || slotIndex >= m_progressStatus.size())
        return;
    m_progressStatus[slotIndex]->setText(statusText);
    m_progressStatus[slotIndex]->setStyleSheet(statusStyle(statusText));
    if (m_viewModel)
        m_viewModel->slot_updateProgressStatus(slotIndex, statusText);
}

void HomeView::appendLog(const QString &text, const QString &level)
{
    if (!m_logEdit) return;
    const QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString color = QStringLiteral("#dbe4ee");
    if (level == QLatin1String("INFO"))
        color = QStringLiteral("#4caf50");
    else if (level == QLatin1String("WARN"))
        color = QStringLiteral("#ffc107");
    else if (level == QLatin1String("ERROR"))
        color = QStringLiteral("#f44336");
    m_logEdit->append(
        QStringLiteral("<span style='color:%1;'>[%2] %3 %4</span>")
            .arg(color, time, level, text.toHtmlEscaped()));
}

// =============================================================================
// 检测工步运行时配置（写入 FlowContext GLOBAL；仅 slot_initialize 触发）
// =============================================================================

bool HomeView::setFlashDetectStepConfig(const SystemConfig &cfg)
{
    bool masterEnabled = true;
    for (const DetectItemColorSwitches &item : cfg.detectItems) {
        if (item.item == DetectItemKind::Flash) {
            masterEnabled = item.masterEnabled;
            break;
        }
    }

    json flashCfg = json::object();
    flashCfg["enabled"] = masterEnabled;
    setGlobalFlowContextVar(KEY_FLASH_STEP_CONFIG, std::move(flashCfg));
    return true;
}

bool HomeView::setScanStepConfig(const SystemConfig &cfg)
{
    json scanCfg = json::object();
    scanCfg["enabled"] = cfg.generalSwitches.scanBarcode;
    setGlobalFlowContextVar(KEY_SCAN_STEP_CONFIG, std::move(scanCfg));
    return true;
}

bool HomeView::setUpstreamDockStepConfig(const SystemConfig &cfg)
{
    json upstreamCfg = json::object();
    upstreamCfg["enabled"] = cfg.interfaceSwitches.upstreamEnabled;
    setGlobalFlowContextVar(KEY_UPSTREAM_DOCK_STEP_CONFIG, std::move(upstreamCfg));
    return true;
}

bool HomeView::setDownstreamDockStepConfig(const SystemConfig &cfg)
{
    json downstreamCfg = json::object();
    downstreamCfg["enabled"] = cfg.interfaceSwitches.downstreamEnabled;
    setGlobalFlowContextVar(KEY_DOWNSTREAM_DOCK_STEP_CONFIG, std::move(downstreamCfg));
    return true;
}

bool HomeView::setLightDetectStepConfig(const SystemConfig &cfg, const RecipeDocument &recipe)
{
    // 相机列表用于按序构建各相机算法参数（cameraName 须与 procedure_config 中 cameraList 一致）
    if (m_cameraMap.empty()) {
        m_cameraMap = DeviceConfigReader::loadCameraMap();
    }
    if (m_cameraMap.empty()) {
        qWarning() << "[HomeView] no cameras configured in device_config.json";
        return false;
    }

    json camerasAlgo = json::object();
    int cameraIndex = 0;
    for (const auto &entry : m_cameraMap) {
        const common::algorithm::LightenDetectAlgoParams algoParams =
            lightenDetectAlgoParamsFromRecipe(cfg, recipe, cameraIndex);
        json algoJson;
        toJson(algoJson, algoParams);
        camerasAlgo[entry.first] = std::move(algoJson);
        ++cameraIndex;
    }

    json featureFlagsJson;
    toJson(featureFlagsJson, featureFlagsFromSystemConfig(cfg));

    json lightDetectCfg = json::object();
    lightDetectCfg["lightDelayMs"] = cfg.generalSettings.lightDelayMs;
    lightDetectCfg["featureFlags"] = std::move(featureFlagsJson);
    lightDetectCfg["cameras"] = std::move(camerasAlgo);

    setGlobalFlowContextVar(KEY_LIGHT_DETECT_STEP_CONFIG, std::move(lightDetectCfg));
    return true;
}

bool HomeView::setResultSummaryStepConfig(const SystemConfig &cfg, const RecipeDocument &recipe)
{
    const common::algorithm::DefectCountLimits limits = defectCountLimitsFromSystemConfig(cfg);
    const common::algorithm::SpecialDetectRules specialRules =
        specialDetectRulesFromRecipeDocument(recipe);

    json limitsJson;
    toJson(limitsJson, limits);

    json specialRulesJson;
    toJson(specialRulesJson, specialRules);

    common::algorithm::ResultSummaryPanelCoord panelCoord{};
    panelCoord.width              = recipe.basic.width;
    panelCoord.height             = recipe.basic.height;
    panelCoord.coord_system_index = recipe.basic.coordSystemIndex;
    json panelCoordJson;
    toJson(panelCoordJson, panelCoord);

    json summaryCfg = json::object();
    summaryCfg["defectCountLimits"]  = std::move(limitsJson);
    summaryCfg["specialDetectRules"] = std::move(specialRulesJson);
    summaryCfg["panelCoord"]         = std::move(panelCoordJson);

    setGlobalFlowContextVar(KEY_RESULT_SUMMARY_STEP_CONFIG, std::move(summaryCfg));
    return true;
}

bool HomeView::setProductModelFlowContext(const QString &defaultRecipeId)
{
    if (defaultRecipeId.trimmed().isEmpty())
        return false;
    setGlobalFlowContextVar(KEY_PRODUCT_MODEL, defaultRecipeId.trimmed().toStdString());
    return true;
}

bool HomeView::buildStepConfigFromSystemConfig()
{
    const SystemConfig cfg = SystemConfigRepository::load();

    RecipeDocument recipe;
    const QString defaultId = RecipeRepository::defaultRecipeId();
    if (!RecipeRepository::load(defaultId, recipe))
        recipe = RecipeDocument::defaultDemo(defaultId);

    return setProductModelFlowContext(defaultId)
        && setScanStepConfig(cfg)
        && setUpstreamDockStepConfig(cfg)
        && setDownstreamDockStepConfig(cfg)
        && setFlashDetectStepConfig(cfg)
        && setLightDetectStepConfig(cfg, recipe)
        && setResultSummaryStepConfig(cfg, recipe);
}

void HomeView::setupViewModel()
{
    if (!m_viewModel) {
        m_viewModel = new HomeViewModel(this);
    }

    connect(m_viewModel, &HomeViewModel::sig_runtimeChanged, this, [this]() {
        applyRuntimeToUi(m_viewModel->runtime());
    });
    connect(RecipeCatalogNotifier::instance(), &RecipeCatalogNotifier::sig_defaultRecipeChanged,
            m_viewModel, &HomeViewModel::slot_updateCurrentRecipe);

    applyRuntimeToUi(m_viewModel->runtime());
}

void HomeView::resetInspectionSnapshot()
{
    if (!m_viewModel)
        setupViewModel();

    HomeRuntimeState rt = m_viewModel->runtime();
    rt.snapshot = CurrentInspectionSnapshot::defaultEmpty();
    m_viewModel->setRuntime(rt);
}

void HomeView::resetDetectProgress()
{
    if (!m_viewModel)
        setupViewModel();

    HomeRuntimeState rt = m_viewModel->runtime();
    rt.progress.resize(kHomeProgressCount);
    for (int i = 0; i < kHomeProgressCount; ++i)
        rt.progress[i].statusText = QStringLiteral("未开始");
    m_viewModel->setRuntime(rt);
}

} // namespace lighten
