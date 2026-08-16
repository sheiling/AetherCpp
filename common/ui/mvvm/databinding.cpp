#include "databinding.h"

#include "binding_internal.h"
#include "command.h"
#include "uibase/permission/permservice.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QSignalBlocker>
#include <QDebug>

#include <functional>
#include <memory>

// 绑定前检查：空指针与属性存在性（避免静默失败）
static bool checkBinding(ViewModel *viewModel, const QString &propertyName)
{
    if (!viewModel)
        return false;
    if (!viewModel->hasProperty(propertyName)) {
#ifdef QT_DEBUG
        qWarning("DataBinding: ViewModel has no property '%s'", qPrintable(propertyName));
#endif
        return false;
    }
    return true;
}

// ============================================================================
// 单向绑定：ViewModel -> View
// ============================================================================

void DataBinding::bindOneWay(ViewModel *viewModel, const QString &propertyName, QLabel *label)
{
    if (!label || !checkBinding(viewModel, propertyName)) return;

    // 初始化
    label->setText(viewModel->getProperty(propertyName).toString());

    // 监听属性变更
    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    label, [label, propertyName](const QString &name, const QVariant &value) {
        if (name == propertyName) {
            label->setText(value.toString());
        }
    });
}

void DataBinding::bindOneWay(ViewModel *viewModel, const QString &propertyName, QPushButton *button)
{
    if (!button || !checkBinding(viewModel, propertyName)) return;

    // 初始化
    button->setText(viewModel->getProperty(propertyName).toString());

    // 监听属性变更
    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    button, [button, propertyName](const QString &name, const QVariant &value) {
        if (name == propertyName) {
            button->setText(value.toString());
        }
    });
}

void DataBinding::bindOneWay(ViewModel *viewModel, const QString &propertyName, QProgressBar *progressBar)
{
    if (!progressBar || !checkBinding(viewModel, propertyName)) return;

    // 初始化
    progressBar->setValue(viewModel->getProperty(propertyName).toInt());

    // 监听属性变更
    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    progressBar, [progressBar, propertyName](const QString &name, const QVariant &value) {
        if (name == propertyName) {
            progressBar->setValue(value.toInt());
        }
    });
}

// ============================================================================
// 双向绑定：ViewModel <-> View
// ============================================================================

using namespace BindingInternal;

BindingHandle *DataBinding::bind(ViewModel *viewModel, const QString &propertyName,
                                 QLineEdit *lineEdit, const BindingOptions &options,
                                 BindingSession *session)
{
    if (!lineEdit || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [lineEdit](const QVariant &value) {
        QSignalBlocker blocker(lineEdit);
        lineEdit->setText(value.toString());
    };
    const auto pullFromView = [viewModel, propertyName, lineEdit]() {
        viewModel->setProperty(propertyName, lineEdit->text());
    };

    connectVmToView(viewModel, propertyName, lineEdit, applyToView, session);
    connectViewToVm(lineEdit, viewModel, options, pullFromView);
    return registerPullHandle(session, lineEdit, pullFromView);
}

BindingHandle *DataBinding::bind(ViewModel *viewModel, const QString &propertyName,
                                 QSpinBox *spinBox, const BindingOptions &options,
                                 BindingSession *session)
{
    if (!spinBox || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [spinBox](const QVariant &value) {
        QSignalBlocker blocker(spinBox);
        spinBox->setValue(value.toInt());
    };
    const auto pullFromView = [viewModel, propertyName, spinBox]() {
        viewModel->setProperty(propertyName, spinBox->value());
    };

    connectVmToView(viewModel, propertyName, spinBox, applyToView, session);
    connectViewToVm(spinBox, viewModel, options, pullFromView);
    return registerPullHandle(session, spinBox, pullFromView);
}

BindingHandle *DataBinding::bind(ViewModel *viewModel, const QString &propertyName,
                                 QDoubleSpinBox *spinBox, const BindingOptions &options,
                                 BindingSession *session)
{
    if (!spinBox || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [spinBox](const QVariant &value) {
        QSignalBlocker blocker(spinBox);
        spinBox->setValue(value.toDouble());
    };
    const auto pullFromView = [viewModel, propertyName, spinBox]() {
        viewModel->setProperty(propertyName, spinBox->value());
    };

    connectVmToView(viewModel, propertyName, spinBox, applyToView, session);
    connectViewToVm(spinBox, viewModel, options, pullFromView);
    return registerPullHandle(session, spinBox, pullFromView);
}

BindingHandle *DataBinding::bind(ViewModel *viewModel, const QString &propertyName, QSlider *slider,
                                 const BindingOptions &options, BindingSession *session)
{
    if (!slider || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [slider](const QVariant &value) {
        QSignalBlocker blocker(slider);
        slider->setValue(value.toInt());
    };
    const auto pullFromView = [viewModel, propertyName, slider]() {
        viewModel->setProperty(propertyName, slider->value());
    };

    connectVmToView(viewModel, propertyName, slider, applyToView, session);
    connectViewToVm(slider, viewModel, options, pullFromView);
    return registerPullHandle(session, slider, pullFromView);
}

BindingHandle *DataBinding::bind(ViewModel *viewModel, const QString &propertyName,
                                 QCheckBox *checkBox, const BindingOptions &options,
                                 BindingSession *session)
{
    if (!checkBox || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [checkBox](const QVariant &value) {
        QSignalBlocker blocker(checkBox);
        checkBox->setChecked(value.toBool());
    };
    const auto pullFromView = [viewModel, propertyName, checkBox]() {
        viewModel->setProperty(propertyName, checkBox->isChecked());
    };

    connectVmToView(viewModel, propertyName, checkBox, applyToView, session);
    connectViewToVm(checkBox, viewModel, options, pullFromView);
    return registerPullHandle(session, checkBox, pullFromView);
}

BindingHandle *DataBinding::bind(ViewModel *viewModel, const QString &propertyName,
                                 QRadioButton *radioButton, const BindingOptions &options,
                                 BindingSession *session)
{
    if (!radioButton || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [radioButton](const QVariant &value) {
        QSignalBlocker blocker(radioButton);
        radioButton->setChecked(value.toBool());
    };
    const auto pullFromView = [viewModel, propertyName, radioButton]() {
        viewModel->setProperty(propertyName, radioButton->isChecked());
    };

    connectVmToView(viewModel, propertyName, radioButton, applyToView, session);
    connectViewToVm(radioButton, viewModel, options, pullFromView);
    return registerPullHandle(session, radioButton, pullFromView);
}

BindingHandle *DataBinding::bind(ViewModel *viewModel, const QString &propertyName,
                                 QComboBox *comboBox, const BindingOptions &options,
                                 BindingSession *session)
{
    if (!comboBox || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [comboBox](const QVariant &value) {
        QSignalBlocker blocker(comboBox);
        comboBox->setCurrentText(value.toString());
    };
    const auto pullFromView = [viewModel, propertyName, comboBox]() {
        viewModel->setProperty(propertyName, comboBox->currentText());
    };

    connectVmToView(viewModel, propertyName, comboBox, applyToView, session);
    connectViewToVm(comboBox, viewModel, options, pullFromView);
    return registerPullHandle(session, comboBox, pullFromView);
}

BindingHandle *DataBinding::bindIndex(ViewModel *viewModel, const QString &propertyName,
                                      QComboBox *comboBox, const BindingOptions &options,
                                      BindingSession *session)
{
    if (!comboBox || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [comboBox](const QVariant &value) {
        QSignalBlocker blocker(comboBox);
        comboBox->setCurrentIndex(value.toInt());
    };
    const auto pullFromView = [viewModel, propertyName, comboBox]() {
        viewModel->setProperty(propertyName, comboBox->currentIndex());
    };

    connectVmToView(viewModel, propertyName, comboBox, applyToView, session);
    if (options.mode == BindingMode::TwoWay) {
        switch (options.trigger) {
        case UpdateSourceTrigger::OnChanged:
            QObject::connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                             viewModel, [pullFromView](int) { pullFromView(); });
            break;
        case UpdateSourceTrigger::OnLostFocus:
        case UpdateSourceTrigger::Manual:
            break;
        }
    }
    return registerPullHandle(session, comboBox, pullFromView);
}

BindingHandle *DataBinding::bind(ViewModel *viewModel, const QString &propertyName,
                                 QTextEdit *textEdit, const BindingOptions &options,
                                 BindingSession *session)
{
    if (!textEdit || !checkBinding(viewModel, propertyName))
        return nullptr;

    const auto applyToView = [textEdit](const QVariant &value) {
        QSignalBlocker blocker(textEdit);
        textEdit->setPlainText(value.toString());
    };
    const auto pullFromView = [viewModel, propertyName, textEdit]() {
        viewModel->setProperty(propertyName, textEdit->toPlainText());
    };

    connectVmToView(viewModel, propertyName, textEdit, applyToView, session);
    connectViewToVm(textEdit, viewModel, options, pullFromView);
    return registerPullHandle(session, textEdit, pullFromView);
}

void DataBinding::bind(ViewModel *viewModel, const QString &propertyName, QDateEdit *dateEdit)
{
    if (!dateEdit || !checkBinding(viewModel, propertyName)) return;

    // ViewModel -> View
    QDate initDate = viewModel->getProperty(propertyName).toDate();
    if (initDate.isValid())
        dateEdit->setDate(initDate);

    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    dateEdit, [dateEdit, propertyName](const QString &name, const QVariant &value) {
        if (name == propertyName) {
            QDate d = value.toDate();
            if (d.isValid())
                dateEdit->setDate(d);
        }
    });

    // View -> ViewModel
    QObject::connect(dateEdit, &QDateEdit::dateChanged,
                    viewModel, [viewModel, propertyName](const QDate &date) {
        viewModel->setProperty(propertyName, date);
    });
}

void DataBinding::bind(ViewModel *viewModel, const QString &propertyName, QTimeEdit *timeEdit)
{
    if (!timeEdit || !checkBinding(viewModel, propertyName)) return;

    // ViewModel -> View
    QTime initTime = viewModel->getProperty(propertyName).toTime();
    if (initTime.isValid())
        timeEdit->setTime(initTime);

    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    timeEdit, [timeEdit, propertyName](const QString &name, const QVariant &value) {
        if (name == propertyName) {
            QTime t = value.toTime();
            if (t.isValid())
                timeEdit->setTime(t);
        }
    });

    // View -> ViewModel
    QObject::connect(timeEdit, &QTimeEdit::timeChanged,
                    viewModel, [viewModel, propertyName](const QTime &time) {
        viewModel->setProperty(propertyName, time);
    });
}

void DataBinding::bind(ViewModel *viewModel, const QString &propertyName, QDateTimeEdit *dateTimeEdit)
{
    if (!dateTimeEdit || !checkBinding(viewModel, propertyName)) return;

    // ViewModel -> View
    QDateTime initDt = viewModel->getProperty(propertyName).toDateTime();
    if (initDt.isValid())
        dateTimeEdit->setDateTime(initDt);

    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    dateTimeEdit, [dateTimeEdit, propertyName](const QString &name, const QVariant &value) {
        if (name == propertyName) {
            QDateTime dt = value.toDateTime();
            if (dt.isValid())
                dateTimeEdit->setDateTime(dt);
        }
    });

    // View -> ViewModel
    QObject::connect(dateTimeEdit, &QDateTimeEdit::dateTimeChanged,
                    viewModel, [viewModel, propertyName](const QDateTime &dt) {
        viewModel->setProperty(propertyName, dt);
    });
}

// ============================================================================
// 状态绑定
// ============================================================================

void DataBinding::bindEnabled(ViewModel *viewModel, const QString &propertyName, QWidget *widget)
{
    if (!widget || !checkBinding(viewModel, propertyName)) return;

    widget->setEnabled(viewModel->getProperty(propertyName).toBool());
    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    widget, [widget, propertyName](const QString &name, const QVariant &value) {
        if (name == propertyName)
            widget->setEnabled(value.toBool());
    });
}

void DataBinding::bindVisible(ViewModel *viewModel, const QString &propertyName, QWidget *widget)
{
    if (!widget || !checkBinding(viewModel, propertyName)) return;

    widget->setVisible(viewModel->getProperty(propertyName).toBool());
    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    widget, [widget, propertyName](const QString &name, const QVariant &value) {
        if (name == propertyName)
            widget->setVisible(value.toBool());
    });
}

// ============================================================================
// 命令绑定
// ============================================================================

void DataBinding::bindCommand(Command *command, QPushButton *button,
                              const std::function<void()> &beforeRun)
{
    if (!command || !button)
        return;

    const auto syncEnabled = [button, command]() { button->setEnabled(command->canExecute()); };

    syncEnabled();

    QObject::connect(command, &Command::canExecuteChanged, button, syncEnabled);

    if (auto *fc = qobject_cast<FunctionalCommand *>(command)) {
        QObject::connect(PermService::instance(), &PermService::permissionsChanged, command,
                         [fc, syncEnabled]() {
                             fc->updateCanExecute();
                             syncEnabled();
                         });
    } else {
        QObject::connect(PermService::instance(), &PermService::permissionsChanged, command,
                         syncEnabled);
    }

    if (beforeRun) {
        QObject::connect(button, &QPushButton::clicked, command, [command, beforeRun]() {
            beforeRun();
            command->run();
        });
    } else {
        QObject::connect(button, &QPushButton::clicked, command, &Command::run);
    }
}

// ============================================================================
// 通用绑定
// ============================================================================

void DataBinding::bindProperty(ViewModel *viewModel, const QString &vmProperty,
                               QObject *widget, const QString &widgetProperty)
{
    if (!widget || !checkBinding(viewModel, vmProperty)) return;

    // ViewModel -> View (单向绑定)
    QVariant value = viewModel->getProperty(vmProperty);
    widget->setProperty(widgetProperty.toUtf8().constData(), value);

    QObject::connect(viewModel, &ViewModel::propertyChanged,
                    widget, [widget, vmProperty, widgetProperty](const QString &name, const QVariant &value) {
        if (name == vmProperty) {
            widget->setProperty(widgetProperty.toUtf8().constData(), value);
        }
    });

    // 注意：View -> ViewModel 的双向绑定需要手动实现
    // 因为通用属性的信号连接比较复杂，建议使用专用的 bind 方法
}
