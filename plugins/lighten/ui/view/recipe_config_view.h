#pragma once

/**
 * @file recipe_config_view.h
 * @brief 产品配方配置视图（配方列表、相机 ROI、算法阈值、特殊检测）
 */

#include "pagewidget.h"
#include "lighten_param_persist.h"

#include <QImage>
#include <QListWidget>
#include <QPointer>
#include <QVariantList>

#include <string>
#include <vector>

class QDialog;
class QButtonGroup;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QDoubleSpinBox;
class QLabel;
class QStackedWidget;
class QTableWidget;
class QTextEdit;
class QWidget;

class IGraphicsVew;

namespace ui {
class ThemedIconButton;
}

namespace lighten {

class RecipeConfigViewModel;

/** 相机设置表内可编辑控件（当前 Tab 对应 document.cameras[activeCamera]） */
struct RecipeCameraSettingsWidgets
{
    QSpinBox  *pureExposureR   = nullptr;
    QSpinBox  *pureExposureG   = nullptr;
    QSpinBox  *pureExposureB   = nullptr;
    QSpinBox  *pureGrayR       = nullptr;
    QSpinBox  *pureGrayG       = nullptr;
    QSpinBox  *pureGrayB       = nullptr;
    QSpinBox  *seriesExposureR = nullptr;
    QSpinBox  *seriesExposureG = nullptr;
    QSpinBox  *seriesExposureB = nullptr;
    QSpinBox  *seriesGrayR     = nullptr;
    QSpinBox  *seriesGrayG     = nullptr;
    QSpinBox  *seriesGrayB     = nullptr;
    QSpinBox  *whiteExposure   = nullptr;
    QSpinBox  *blackExposure   = nullptr;
    QLabel    *qualityR        = nullptr;
    QLabel    *qualityG        = nullptr;
    QLabel    *qualityB        = nullptr;
    QComboBox *rotation        = nullptr;
};

/** 算法阈值表单行 RGB 阈值 SpinBox 聚合 */
struct RecipeAlgoRgbRowWidgets
{
    QDoubleSpinBox *r = nullptr;
    QDoubleSpinBox *g = nullptr;
    QDoubleSpinBox *b = nullptr;
};

/** 算法阈值表全部行控件聚合 */
struct RecipeAlgoThresholdWidgets
{
    static constexpr int kRowCount = 9;
    RecipeAlgoRgbRowWidgets rows[kRowCount];
    QDoubleSpinBox       *blackScreenThresh       = nullptr;
    QDoubleSpinBox       *blackScreenBrightThresh = nullptr;
    QDoubleSpinBox       *badPixelThresh          = nullptr;
};

/** 特殊检测表单行控件聚合 */
struct RecipeSpecialDetectRowWidgets
{
    QWidget  *switchCell    = nullptr;
    QSpinBox *lampThreshold = nullptr;
    QComboBox *action       = nullptr;
};

/** 特殊检测表全部行控件聚合 */
struct RecipeSpecialDetectWidgets
{
    static constexpr int kRowCount = 4;
    RecipeSpecialDetectRowWidgets rows[kRowCount];
};

/** 结果输出区单个芯片内的 R/G/B/总数标签句柄 */
struct RecipeResultChipLabels
{
    QLabel *r     = nullptr;
    QLabel *g     = nullptr;
    QLabel *b     = nullptr;
    QLabel *total = nullptr;
};

/**
 * @brief 产品配方配置视图（布局对齐产品配方配置目标稿）
 *
 * 顶栏两行：操作按钮行 + 状态行；左为配方列表（搜索 + 名称/状态分列）；
 * 中为相机与 ROI 工具条、主预览与右侧纵向单步调试、底部结果输出；
 * 右为可折叠参数区（基本信息默认展开，其余默认收起）。
 */
class RecipeConfigView : public PageWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigView(QWidget *parent = nullptr);
    ~RecipeConfigView() override;

protected:
    void onLoad() override;
    void applyTheme() override;

private slots:
    void slot_camera_img(const QVariantList &args);
    void slot_camera_state(const QVariantList &args);
    void slot_holdRoiSaveClicked();
    void slot_viewRoiClicked();
    void slot_stepFeedInClicked();
    void slot_stepFeedOutClicked();
    void slot_stepReadImageClicked();
    void slot_stepPureColorDetectClicked();
    void slot_stepStringLightDetectClicked();
    void slot_stepMixColorDetectClicked();
    void slot_stepBlackScreenLightDetectClicked();
    void slot_stepFlashDetectClicked();
    /** EVENT_DEBUG_NORMAL_DEFECT_COUNT：算法 Notify 后刷新结果输出区芯片（主线程） */
    void slot_debug_pure_color_defect_count(const QVariantList &args);
    /** 结果输出区芯片点击（参数为检测项类型） */
    void slot_resultChipClicked(DetectItemKind kind);

private:
    void applyRecipeTheme();
    void buildLayout();

    QWidget *buildTopBar();
    QWidget *buildRecipeList();
    QWidget *buildCenterPanel(const std::vector<std::string> &cameraKeys);
    QWidget *buildParamPanel(const std::vector<std::string> &cameraKeys);

    QWidget *buildParamBasicInfo(QWidget *parent);
    QWidget *buildParamCameraSettings(QWidget *parent, const std::vector<std::string> &cameraKeys);
    QWidget *buildParamAlgoThresholds(QWidget *parent);
    QWidget *buildParamSpecialDetection(QWidget *parent);
    QWidget *buildParamOtherSettings(QWidget *parent);

    QWidget *makeCollapsible(const QString &title, QWidget *content, QWidget *parent,
                             bool expanded = true);

    QTableWidget *buildCameraSettingsTable(QWidget *parent);
    void connectCameraTabComboSync();
    void setupViewModel();
    void setupBindings();
    void bindTopBarCommands();
    void appendLog(const QString &text, const QString &level = QStringLiteral("INFO"));
    /** 将图像显示到预览区；无效图像时回退占位页 */
    void showPreviewImage(const QImage &image);
    /** 选择 RAW 文件并在后台线程解码，经 CAM_ASYNC_EVENT_CAMERA_IMG 显示；失败后写日志 */
    void loadRawImageFromDialog(QString &fileName);
    /** 将 r/g/b 写入指定检测项芯片标签，总数 = r + g + b */
    void applyResultChipLabels(DetectItemKind kind, int r, int g, int b);
    /** 单步调试前：清零结果输出区全部芯片（document + UI），并删除 sample 目录下除 *.contour 外的文件 */
    void resetStepDebugResultOutput();
    /** 单步调试「纯色检测」：校验颜色通道后，在线程中采图/读图并执行算法（含串色） */
    void triggerStepDebugPureColorDetect();
    /** 单步调试「串灯检测」：校验轮廓与颜色通道后，在线程中采图/读图并执行算法 */
    void triggerStepDebugStringLightDetect();
    /** 填充单步调试共用相机上下文；失败时写日志并返回 false */
    bool tryFillStepDebugCameraContext(QString *cameraName,
                                       QString *cameraPath,
                                       int *xResolution,
                                       int *yResolution,
                                       int *elemSize) const;
    /** 将工具栏颜色按钮当前选中项写入 ViewModel::activeColorChannel */
    void syncToolbarColorChannelFromUi();
    /** 结果芯片 sample 图路径（暂未实现） */
    QString resultChipSampleImagePath(DetectItemKind kind) const;
    void showSaveSuccessTip();
    bool isRecipeListSelectionValid() const;
    /** 无边框主题弹窗输入配方名称；取消返回 false */
    bool promptRecipeName(QString *outName, const QString &defaultName,
                          const QString &dialogTitle = QStringLiteral("新增配方")) const;
    bool confirmRemoveRecipe() const;
    void syncCopyRecipeButtonEnabled();
    void syncRemoveRecipeButtonEnabled();
    void bindCameraSettings();
    void bindAlgoThresholds();
    void bindSpecialDetection();
    void bindOtherSettings();
    void applyCameraSettingsToUi();
    /** 首相机全部可编辑；其余相机仅纯色曝光/灰阶、旋转角度可编辑（图像质量行暂隐藏） */
    void applyCameraSettingsEditability(bool isFirstCamera);
    void applyAlgoThresholdsToUi();
    void applySpecialDetectionToUi();
    void applyOtherSettingsToUi();
    void applyStatusBarToUi();
    void refreshRecipeList();
    /** 将预览区当前 ROI 写入 document.debugs（当前相机+颜色通道） */
    void capturePreviewRoiToDocument();
    /** 按工具栏 ROI 模式下拉项刷新预览区绘制状态 */
    void applyToolbarRoiMode(int index);
    /** 按「区域定位」开关同步 ROI 模式下拉可编辑状态 */
    void syncRoiModeComboEnabled(bool roiEnabled);
    /** 配方切换后：相机下拉默认第一项并触发一次变更逻辑 */
    void resetToolbarCameraOnRecipeSwitch();
    /** 工具栏相机切换：ROI 模式复位为第一项，预览区显示占位页 */
    void onToolbarCameraChanged(int index);
    /** 按当前选中相机的 GetCaptureState 同步预览/抓拍按钮 */
    void syncPreviewButtonFromCaptureState();

    RecipeConfigViewModel *m_viewModel             = nullptr;
    QPointer<QDialog>      m_saveSuccessTipDlg;
    QPushButton           *m_addRecipeBtn          = nullptr;
    QPushButton           *m_copyRecipeBtn         = nullptr;
    QPushButton           *m_removeRecipeBtn       = nullptr;
    QPushButton           *m_setDefaultRecipeBtn   = nullptr;
    QPushButton           *m_importRecipeBtn       = nullptr;
    QPushButton           *m_exportRecipeBtn       = nullptr;
    QPushButton           *m_saveRecipeBtn         = nullptr;
    QLineEdit             *m_recipeSearchEdit      = nullptr;
    QListWidget           *m_recipeList            = nullptr;
    QTextEdit             *m_logEdit                 = nullptr;
    QLabel                *m_statusRecipeLbl       = nullptr;
    QLabel                *m_statusQualityLbl      = nullptr;
    QLabel                *m_statusParamLbl        = nullptr;
    QLineEdit             *m_basicNameEdit         = nullptr;
    QSpinBox              *m_basicWidthSpin        = nullptr;
    QSpinBox              *m_basicHeightSpin       = nullptr;
    QComboBox             *m_basicCoordCombo       = nullptr;
    QComboBox             *m_toolbarCamCombo       = nullptr;
    QPointer<ui::ThemedIconButton> m_liveBtn        = nullptr;
    QPointer<ui::ThemedIconButton> m_snapBtn        = nullptr;
    QButtonGroup          *m_colorBtnGroup         = nullptr;
    QWidget               *m_roiSwitch             = nullptr;
    QComboBox             *m_roiModeCombo          = nullptr;
    QPushButton           *m_holdRoiBtn            = nullptr;
    QPushButton           *m_viewRoiBtn            = nullptr;
    IGraphicsVew          *m_graphicsView          = nullptr;
    QStackedWidget        *m_previewStack          = nullptr;
    QButtonGroup          *m_camSettingsTabGroup   = nullptr;
    RecipeCameraSettingsWidgets m_camSettingsWidgets;
    RecipeAlgoThresholdWidgets  m_algoWidgets;
    RecipeSpecialDetectWidgets  m_specialDetectWidgets;
    QDoubleSpinBox             *m_otherTrackWidthSpin = nullptr;
    bool                        m_syncingCameraSettings = false;
    bool                        m_syncingExtraParams    = false;
    /** 单步调试「读取图像」本地 RAW 路径；纯色/串灯检测成功后清空，失败时保留 */
    QString                     m_stepDebugMainRawPath;
    /** 结果输出区 8 个芯片标签，下标与 resultChipSlotIndex(DetectItemKind) 一致 */
    std::array<RecipeResultChipLabels, 8> m_resultChipLabels{};
};

} // namespace lighten
