/**
 * @file recipe_config_camera_view.cpp
 * @brief 配方配置 · 相机设置子面板实现
 */

#include "recipe_config/recipe_config_camera_view.h"

#include "component/controls/slider/themed_slider.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/spinbox/themed_spinbox.h"
#include "recipe_config_view_model.h"
#include "binding_internal.h"
#include "binding_handle.h"
#include "binding_options.h"
#include "databinding.h"

#include "uibase/languagemanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

RecipeConfigCameraView::RecipeConfigCameraView(RecipeConfigViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    buildLayout();
}

void RecipeConfigCameraView::buildLayout()
{
    auto *vl = new QVBoxLayout(this);
    vl->setContentsMargins(8, 2, 8, 8);
    vl->setSpacing(6);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.camera.light_brightness"), this));
    auto *sliderRow = new QHBoxLayout;
    m_lightSlider = makeHorizontalSlider(this);
    m_lightSlider->setObjectName(QStringLiteral("recipeLightSlider"));
    m_lightSlider->setRange(0, 255);
    m_lightValueLabel = new QLabel(QStringLiteral("0"), this);
    m_lightValueLabel->setObjectName(QStringLiteral("recipeSliderValue"));
    connect(m_lightSlider, &ui::ThemedSlider::valueChanged, m_lightValueLabel,
            [this](int v) { m_lightValueLabel->setText(QString::number(v)); });
    sliderRow->addWidget(m_lightSlider, 1);
    sliderRow->addWidget(m_lightValueLabel);
    vl->addLayout(sliderRow);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.camera.exposure_us"), this));
    m_exposureSpin = makeSpinBox(this, QStringLiteral("recipeCamSpin"));
    m_exposureSpin->setRange(1, 9999999);
    m_exposureSpin->setSingleStep(1000);
    vl->addWidget(m_exposureSpin);

    auto *imageSizeBlock = new QWidget(this);
    auto *imageSizeVl    = new QVBoxLayout(imageSizeBlock);
    imageSizeVl->setContentsMargins(0, 0, 0, 0);
    imageSizeVl->setSpacing(2);

    auto *imageLabelRow = new QWidget(imageSizeBlock);
    auto *imageLabelHl  = new QHBoxLayout(imageLabelRow);
    imageLabelHl->setContentsMargins(0, 0, 0, 0);
    imageLabelHl->setSpacing(6);
    imageLabelHl->addWidget(
        makeFormLabel(QStringLiteral("aoi.recipe.camera.image_width"), imageLabelRow), 1);
    imageLabelHl->addWidget(
        makeFormLabel(QStringLiteral("aoi.recipe.camera.image_height"), imageLabelRow), 1);
    imageSizeVl->addWidget(imageLabelRow);

    auto *imageSpinRow = new QWidget(imageSizeBlock);
    auto *imageSpinHl  = new QHBoxLayout(imageSpinRow);
    imageSpinHl->setContentsMargins(0, 0, 0, 0);
    imageSpinHl->setSpacing(6);
    m_imageWidthSpin = makeSpinBox(imageSpinRow, QStringLiteral("recipeCamSpin"));
    m_imageWidthSpin->setRange(1, 16384);
    imageSpinHl->addWidget(m_imageWidthSpin, 1);
    m_imageHeightSpin = makeSpinBox(imageSpinRow, QStringLiteral("recipeCamSpin"));
    m_imageHeightSpin->setRange(1, 16384);
    imageSpinHl->addWidget(m_imageHeightSpin, 1);
    imageSizeVl->addWidget(imageSpinRow);
    vl->addWidget(imageSizeBlock);

    vl->addWidget(makeFormLabel(QStringLiteral("aoi.recipe.camera.capture_accuracy"), this));
    m_captureAccuracySpin = makeDoubleSpinBox(this, QStringLiteral("recipeCamSpin"));
    m_captureAccuracySpin->setRange(0.0, 9999.99);
    m_captureAccuracySpin->setDecimals(2);
    vl->addWidget(m_captureAccuracySpin);
}

void RecipeConfigCameraView::setupBindings()
{
    if (!m_viewModel)
        return;

    using VM = RecipeConfigViewModel;
    ViewModel *const vm   = m_viewModel;
    const QString    prop = VM::PROP_DOCUMENT;

    const BindingOptions onChanged;

    if (m_lightSlider) {
        const auto pullLight = [vm, prop, this]() {
            RecipeDocument d = vm->getProperty(prop).value<RecipeDocument>();
            d.camera.lightBrightness = m_lightSlider->value();
            vm->setProperty(prop, QVariant::fromValue(d));
        };
        BindingInternal::connectStructVmToView(
            vm, prop, m_lightSlider,
            [this](const QVariant &val) {
                QSignalBlocker b(m_lightSlider);
                m_lightSlider->setValue(val.value<RecipeDocument>().camera.lightBrightness);
                if (m_lightValueLabel)
                    m_lightValueLabel->setText(
                        QString::number(val.value<RecipeDocument>().camera.lightBrightness));
            },
            &m_bindings);
        QObject::connect(m_lightSlider, &ui::ThemedSlider::valueChanged, vm,
                         [pullLight](int) { pullLight(); });
        m_bindings.add(std::make_unique<LambdaBindingHandle>(m_lightSlider, pullLight));
    }

    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.camera.exposureUs; },
        [](RecipeDocument &d, int v) { d.camera.exposureUs = v; }, m_exposureSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.camera.imageWidth; },
        [](RecipeDocument &d, int v) { d.camera.imageWidth = v; }, m_imageWidthSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.camera.imageHeight; },
        [](RecipeDocument &d, int v) { d.camera.imageHeight = v; }, m_imageHeightSpin, onChanged,
        &m_bindings);
    DataBinding::bindStructField<RecipeDocument>(
        vm, prop, [](const RecipeDocument &d) { return d.camera.captureAccuracy; },
        [](RecipeDocument &d, double v) { d.camera.captureAccuracy = v; }, m_captureAccuracySpin,
        onChanged, &m_bindings);
}

void RecipeConfigCameraView::retranslatePanel()
{
    ui::ThemedFormLabel::retranslateIn(this);
}

} // namespace aoi
