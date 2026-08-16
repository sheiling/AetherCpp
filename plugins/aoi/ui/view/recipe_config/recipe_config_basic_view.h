#pragma once

#include "binding_session.h"

#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/spinbox/themed_spinbox.h"

#include <QWidget>

class QLabel;

class QDoubleSpinBox;
class QSpinBox;
class QVBoxLayout;

namespace aoi {

class RecipeConfigViewModel;

class RecipeConfigBasicView : public QWidget
{
    Q_OBJECT

public:
    explicit RecipeConfigBasicView(RecipeConfigViewModel *viewModel, QWidget *parent = nullptr);

    void setupBindings();
    void updateAllSources() { m_bindings.updateAllSources(); }
    void beginSuspendUpdates() { m_bindings.beginSuspendUpdates(); }
    void endSuspendUpdates() { m_bindings.endSuspendUpdates(); }
    void retranslatePanel();

private:
    void buildLayout();
    void addPointRow(QVBoxLayout *vl, const QString &labelKey, QDoubleSpinBox *&xSpin,
                     QDoubleSpinBox *&ySpin);

    RecipeConfigViewModel *m_viewModel = nullptr;
    BindingSession         m_bindings;

    QLabel         *m_nameLabel        = nullptr;
    QSpinBox       *m_widthSpin        = nullptr;
    QSpinBox       *m_heightSpin       = nullptr;
    QDoubleSpinBox *m_groupPitchXSpin  = nullptr;
    QDoubleSpinBox *m_groupPitchYSpin  = nullptr;
    QDoubleSpinBox *m_chipPitchSpin    = nullptr;
    QDoubleSpinBox *m_chipWSpin        = nullptr;
    QDoubleSpinBox *m_chipHSpin        = nullptr;
    QDoubleSpinBox *m_padWSpin         = nullptr;
    QDoubleSpinBox *m_padHSpin         = nullptr;
    QDoubleSpinBox *m_groupWSpin       = nullptr;
    QDoubleSpinBox *m_groupHSpin       = nullptr;
    ui::ThemedComboBox *m_colorOrderCombo  = nullptr;
    ui::ThemedComboBox *m_arrangementCombo = nullptr;
    QDoubleSpinBox *m_offsetBlX        = nullptr;
    QDoubleSpinBox *m_offsetBlY        = nullptr;
    QDoubleSpinBox *m_offsetTrX        = nullptr;
    QDoubleSpinBox *m_offsetTrY        = nullptr;
};

} // namespace aoi
