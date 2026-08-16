#pragma once

/**
 * @file recipe_config_widgets.h
 * @brief 配方配置各子面板共用的控件聚合结构体
 */

#include "component/controls/button/themed_push_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/radio/themed_radio_button.h"

class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QSpinBox;

namespace aoi {

/** Mark 采集坐标 XYZ 控件组 */
struct RecipeMarkCaptureWidgets
{
    QLineEdit *x = nullptr;
    QLineEdit *y = nullptr;
    QLineEdit *z = nullptr;
};

/** 单个 Mark 槽位完整表单控件（含模板、类型、匹配阈值、计算结果） */
struct RecipeMarkFormWidgets
{
    RecipeMarkCaptureWidgets capture;
    QLabel         *templateFileLbl  = nullptr;
    QLabel         *templateNameLbl  = nullptr;
    ui::ThemedComboBox *markType         = nullptr;
    QSpinBox       *markWidth        = nullptr;
    QSpinBox       *markHeight       = nullptr;
    QSpinBox       *areaThreshold    = nullptr;
    QDoubleSpinBox *matchMax         = nullptr;
    QDoubleSpinBox *matchMin         = nullptr;
    ui::ThemedRadioButton *bgBlack       = nullptr;
    ui::ThemedRadioButton *bgWhite       = nullptr;
    QLabel         *thumb            = nullptr;
    ui::ThemedPushButton *calcBtn        = nullptr;
    QLabel         *offsetX          = nullptr;
    QLabel         *offsetY          = nullptr;
    QLabel         *calcResultThumb  = nullptr;
};

/** 灯板标定区控件：模式切换、FOV、Gerber 导入 */
struct RecipeCalibrationWidgets
{
    ui::ThemedRadioButton *panelModeRb  = nullptr;
    ui::ThemedRadioButton *gerberModeRb = nullptr;
    ui::ThemedPushButton *fovGenerateBtn     = nullptr;
    QLabel           *fovInfo            = nullptr;
    ui::ThemedComboBox   *pathMode           = nullptr;
    ui::ThemedPushButton *gerberImportBtn    = nullptr;
    QLabel           *gerberFileLbl      = nullptr;
};

} // namespace aoi
