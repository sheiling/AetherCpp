#ifndef BINDING_INTERNAL_H
#define BINDING_INTERNAL_H

/**
 * @file binding_internal.h
 * @brief DataBinding 内部实现：句柄注册、控件读写、UpdateSourceTrigger 信号接线
 *
 * 仅供 databinding.cpp 与单元测试 include；业务 View 应使用 DataBinding 公开 API。
 */

#include "binding_handle.h"
#include "binding_options.h"
#include "binding_session.h"
#include "viewmodel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>

#include <functional>
#include <memory>

namespace BindingInternal {

inline BindingHandle *registerPullHandle(BindingSession *session, QWidget *widget,
                                         const std::function<void()> &pullFromView)
{
    if (!session)
        return nullptr;
    return session->add(std::make_unique<LambdaBindingHandle>(widget, pullFromView));
}

inline void connectVmToView(ViewModel *viewModel, const QString &propertyName, QObject *receiver,
                              const std::function<void(const QVariant &)> &applyToView,
                              BindingSession *session)
{
    applyToView(viewModel->getProperty(propertyName));
    QObject::connect(viewModel, &ViewModel::propertyChanged, receiver,
                     [propertyName, session, applyToView](const QString &name, const QVariant &value) {
                         if (name != propertyName)
                             return;
                         if (session && session->isSuspended())
                             return;
                         applyToView(value);
                     });
}

inline void connectStructVmToView(ViewModel *viewModel, const QString &structProp, QObject *receiver,
                                  const std::function<void(const QVariant &)> &applyToView,
                                  BindingSession *session)
{
    applyToView(viewModel->getProperty(structProp));
    QObject::connect(viewModel, &ViewModel::propertyChanged, receiver,
                     [structProp, session, applyToView](const QString &name, const QVariant &value) {
                         if (name != structProp)
                             return;
                         if (session && session->isSuspended())
                             return;
                         applyToView(value);
                     });
}

inline void connectViewToVm(QObject *widget, ViewModel *viewModel, const BindingOptions &options,
                            const std::function<void()> &pullFromView)
{
    if (options.mode != BindingMode::TwoWay)
        return;

    switch (options.trigger) {
    case UpdateSourceTrigger::OnChanged:
        if (auto *lineEdit = qobject_cast<QLineEdit *>(widget)) {
            QObject::connect(lineEdit, &QLineEdit::textChanged, viewModel,
                             [pullFromView](const QString &) { pullFromView(); });
        } else if (auto *spin = qobject_cast<QSpinBox *>(widget)) {
            QObject::connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), viewModel,
                             [pullFromView](int) { pullFromView(); });
        } else if (auto *dspin = qobject_cast<QDoubleSpinBox *>(widget)) {
            QObject::connect(dspin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), viewModel,
                             [pullFromView](double) { pullFromView(); });
        } else if (auto *slider = qobject_cast<QSlider *>(widget)) {
            QObject::connect(slider, &QSlider::valueChanged, viewModel,
                             [pullFromView](int) { pullFromView(); });
        } else if (auto *check = qobject_cast<QCheckBox *>(widget)) {
            QObject::connect(check, &QCheckBox::toggled, viewModel,
                             [pullFromView](bool) { pullFromView(); });
        } else if (auto *radio = qobject_cast<QRadioButton *>(widget)) {
            QObject::connect(radio, &QRadioButton::toggled, viewModel,
                             [pullFromView](bool) { pullFromView(); });
        } else if (auto *combo = qobject_cast<QComboBox *>(widget)) {
            QObject::connect(combo, &QComboBox::currentTextChanged, viewModel,
                             [pullFromView](const QString &) { pullFromView(); });
        } else if (auto *textEdit = qobject_cast<QTextEdit *>(widget)) {
            QObject::connect(textEdit, &QTextEdit::textChanged, viewModel,
                             [pullFromView]() { pullFromView(); });
        }
        break;
    case UpdateSourceTrigger::OnLostFocus:
        if (auto *lineEdit = qobject_cast<QLineEdit *>(widget)) {
            QObject::connect(lineEdit, &QLineEdit::editingFinished, viewModel,
                             [pullFromView]() { pullFromView(); });
        } else if (auto *spin = qobject_cast<QSpinBox *>(widget)) {
            QObject::connect(spin, &QSpinBox::editingFinished, viewModel,
                             [pullFromView]() { pullFromView(); });
        } else if (auto *dspin = qobject_cast<QDoubleSpinBox *>(widget)) {
            QObject::connect(dspin, &QDoubleSpinBox::editingFinished, viewModel,
                             [pullFromView]() { pullFromView(); });
        }
        break;
    case UpdateSourceTrigger::Manual:
        break;
    }
}

} // namespace BindingInternal

#endif // BINDING_INTERNAL_H
