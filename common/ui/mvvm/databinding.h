#ifndef DATABINDING_H
#define DATABINDING_H

#include "mvvm_global.h"
#include <QObject>
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QSlider>
#include <QProgressBar>
#include <QDateEdit>
#include <QTimeEdit>
#include <QDateTimeEdit>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QMetaMethod>
#include <functional>
#include "binding_handle.h"
#include "binding_internal.h"
#include "binding_options.h"
#include "binding_session.h"
#include "viewmodel.h"
#include "command.h"

/**
 * @brief 数据绑定辅助类
 *
 * - 默认 `BindingMode::TwoWay` + `UpdateSourceTrigger::OnChanged`
 * - VM → View：随 `propertyChanged` 即时推送（不受 trigger 影响）
 * - View → VM：按 `BindingOptions::trigger` 写回；`Manual` 需 `BindingSession::updateAllSources()`
 * - 嵌套结构体字段：`bindStructField` / `bindStructIndexField`（整份 struct 作为 QVariant 属性）
 *
 * @code
 *   BindingSession session;
 *   DataBinding::bind(vm, "userName", lineEdit, {}, &session);
 *   DataBinding::bindStructField<Address>(vm, "address",
 *       [](const Address &a){ return a.city; },
 *       [](Address &a, const QString &v){ a.city = v; }, cityEdit);
 *   session.updateAllSources(); // 保存前 flush Manual 项
 * @endcode
 */
class MVVM_EXPORT DataBinding
{
public:
    // ========== 单向绑定：ViewModel -> View ==========

    // 绑定到QLabel
    static void bindOneWay(ViewModel *viewModel, const QString &propertyName, QLabel *label);

    // 绑定到QPushButton文本
    static void bindOneWay(ViewModel *viewModel, const QString &propertyName, QPushButton *button);

    // 绑定到QProgressBar（单向，int值）
    static void bindOneWay(ViewModel *viewModel, const QString &propertyName, QProgressBar *progressBar);

    // ========== 双向绑定：ViewModel <-> View ==========

    // 绑定QLineEdit
    static BindingHandle *bind(ViewModel *viewModel, const QString &propertyName, QLineEdit *lineEdit,
                               const BindingOptions &options = {}, BindingSession *session = nullptr);

    // 绑定QSpinBox（int）
    static BindingHandle *bind(ViewModel *viewModel, const QString &propertyName, QSpinBox *spinBox,
                               const BindingOptions &options = {}, BindingSession *session = nullptr);

    // 绑定QDoubleSpinBox（double）
    static BindingHandle *bind(ViewModel *viewModel, const QString &propertyName,
                               QDoubleSpinBox *spinBox, const BindingOptions &options = {},
                               BindingSession *session = nullptr);

    // 绑定QSlider（int）
    static BindingHandle *bind(ViewModel *viewModel, const QString &propertyName, QSlider *slider,
                               const BindingOptions &options = {}, BindingSession *session = nullptr);

    // 绑定QCheckBox
    static BindingHandle *bind(ViewModel *viewModel, const QString &propertyName, QCheckBox *checkBox,
                               const BindingOptions &options = {}, BindingSession *session = nullptr);

    // 绑定QRadioButton（bool）
    static BindingHandle *bind(ViewModel *viewModel, const QString &propertyName,
                               QRadioButton *radioButton, const BindingOptions &options = {},
                               BindingSession *session = nullptr);

    // 绑定QComboBox（当前文本）
    static BindingHandle *bind(ViewModel *viewModel, const QString &propertyName, QComboBox *comboBox,
                               const BindingOptions &options = {}, BindingSession *session = nullptr);

    // 绑定QComboBox（当前索引，int）
    static BindingHandle *bindIndex(ViewModel *viewModel, const QString &propertyName,
                                    QComboBox *comboBox, const BindingOptions &options = {},
                                    BindingSession *session = nullptr);

    // 绑定QTextEdit
    static BindingHandle *bind(ViewModel *viewModel, const QString &propertyName, QTextEdit *textEdit,
                               const BindingOptions &options = {}, BindingSession *session = nullptr);

    // 绑定QDateEdit（QDate）
    static void bind(ViewModel *viewModel, const QString &propertyName, QDateEdit *dateEdit);

    // 绑定QTimeEdit（QTime）
    static void bind(ViewModel *viewModel, const QString &propertyName, QTimeEdit *timeEdit);

    // 绑定QDateTimeEdit（QDateTime）
    static void bind(ViewModel *viewModel, const QString &propertyName, QDateTimeEdit *dateTimeEdit);

    // ========== 状态绑定（属性驱动控件状态）==========

    // 绑定控件可用状态（bool属性 -> enabled）
    static void bindEnabled(ViewModel *viewModel, const QString &propertyName, QWidget *widget);

    // 绑定控件可见状态（bool属性 -> visible）
    static void bindVisible(ViewModel *viewModel, const QString &propertyName, QWidget *widget);

    // ========== 命令绑定 ==========

    // 绑定Command到按钮（随 Command::canExecuteChanged 与 PermService::permissionsChanged 刷新可用状态）
    // beforeRun 在 run() 前执行（如 flush Manual 绑定项）
    static void bindCommand(Command *command, QPushButton *button,
                            const std::function<void()> &beforeRun = {});

    // ========== 通用绑定 ==========

    // 通用属性绑定（使用Qt属性系统）
    static void bindProperty(ViewModel *viewModel, const QString &vmProperty,
                            QObject *widget, const QString &widgetProperty);

    // ========== 结构体字段绑定（模板，定义在头文件中）==========
    // 用于将结构体属性中的某个字段绑定到UI控件
    // 结构体类型需要先以 Q_DECLARE_METATYPE 注册，并在 ViewModel 中注册为属性

    /**
     * @brief 绑定结构体字段（QString 字段 <-> QLineEdit）
     * @param getField 从结构体提取字段值的函数
     * @param setField 向结构体写入字段值的函数
     *
     * 示例：
     *   DataBinding::bindStructField<Address>(vm, "address",
     *       [](const Address &a){ return a.city; },
     *       [](Address &a, const QString &v){ a.city = v; }, cityEdit);
     */
    template<typename Struct>
    static BindingHandle *bindStructField(ViewModel *vm, const QString &structProp,
                                          std::function<QString(const Struct &)> getField,
                                          std::function<void(Struct &, const QString &)> setField,
                                          QLineEdit *widget, const BindingOptions &options = {},
                                          BindingSession *session = nullptr)
    {
        if (!widget || !vm || !vm->hasProperty(structProp))
            return nullptr;

        const auto applyToView = [widget, getField](const QVariant &val) {
            QSignalBlocker blocker(widget);
            widget->setText(getField(val.value<Struct>()));
        };
        const auto pullFromView = [vm, structProp, widget, setField]() {
            Struct s = vm->getProperty(structProp).value<Struct>();
            setField(s, widget->text());
            vm->setProperty(structProp, QVariant::fromValue(s));
        };

        BindingInternal::connectStructVmToView(vm, structProp, widget, applyToView, session);
        BindingInternal::connectViewToVm(widget, vm, options, pullFromView);
        return BindingInternal::registerPullHandle(session, widget, pullFromView);
    }

    template<typename Struct>
    static BindingHandle *bindStructField(ViewModel *vm, const QString &structProp,
                                          std::function<int(const Struct &)> getField,
                                          std::function<void(Struct &, int)> setField,
                                          QSpinBox *widget, const BindingOptions &options = {},
                                          BindingSession *session = nullptr)
    {
        if (!widget || !vm || !vm->hasProperty(structProp))
            return nullptr;

        const auto applyToView = [widget, getField](const QVariant &val) {
            QSignalBlocker blocker(widget);
            widget->setValue(getField(val.value<Struct>()));
        };
        const auto pullFromView = [vm, structProp, widget, setField]() {
            Struct s = vm->getProperty(structProp).value<Struct>();
            setField(s, widget->value());
            vm->setProperty(structProp, QVariant::fromValue(s));
        };

        BindingInternal::connectStructVmToView(vm, structProp, widget, applyToView, session);
        BindingInternal::connectViewToVm(widget, vm, options, pullFromView);
        return BindingInternal::registerPullHandle(session, widget, pullFromView);
    }

    template<typename Struct>
    static BindingHandle *bindStructField(ViewModel *vm, const QString &structProp,
                                          std::function<double(const Struct &)> getField,
                                          std::function<void(Struct &, double)> setField,
                                          QDoubleSpinBox *widget, const BindingOptions &options = {},
                                          BindingSession *session = nullptr)
    {
        if (!widget || !vm || !vm->hasProperty(structProp))
            return nullptr;

        const auto applyToView = [widget, getField](const QVariant &val) {
            QSignalBlocker blocker(widget);
            widget->setValue(getField(val.value<Struct>()));
        };
        const auto pullFromView = [vm, structProp, widget, setField]() {
            Struct s = vm->getProperty(structProp).value<Struct>();
            setField(s, widget->value());
            vm->setProperty(structProp, QVariant::fromValue(s));
        };

        BindingInternal::connectStructVmToView(vm, structProp, widget, applyToView, session);
        BindingInternal::connectViewToVm(widget, vm, options, pullFromView);
        return BindingInternal::registerPullHandle(session, widget, pullFromView);
    }

    template<typename Struct>
    static BindingHandle *bindStructField(ViewModel *vm, const QString &structProp,
                                          std::function<bool(const Struct &)> getField,
                                          std::function<void(Struct &, bool)> setField,
                                          QCheckBox *widget, const BindingOptions &options = {},
                                          BindingSession *session = nullptr)
    {
        if (!widget || !vm || !vm->hasProperty(structProp))
            return nullptr;

        const auto applyToView = [widget, getField](const QVariant &val) {
            QSignalBlocker blocker(widget);
            widget->setChecked(getField(val.value<Struct>()));
        };
        const auto pullFromView = [vm, structProp, widget, setField]() {
            Struct s = vm->getProperty(structProp).value<Struct>();
            setField(s, widget->isChecked());
            vm->setProperty(structProp, QVariant::fromValue(s));
        };

        BindingInternal::connectStructVmToView(vm, structProp, widget, applyToView, session);
        BindingInternal::connectViewToVm(widget, vm, options, pullFromView);
        return BindingInternal::registerPullHandle(session, widget, pullFromView);
    }

    template<typename Struct>
    static BindingHandle *bindStructField(ViewModel *vm, const QString &structProp,
                                          std::function<bool(const Struct &)> getField,
                                          std::function<void(Struct &, bool)> setField,
                                          QRadioButton *widget, const BindingOptions &options = {},
                                          BindingSession *session = nullptr)
    {
        if (!widget || !vm || !vm->hasProperty(structProp))
            return nullptr;

        const auto applyToView = [widget, getField](const QVariant &val) {
            QSignalBlocker blocker(widget);
            widget->setChecked(getField(val.value<Struct>()));
        };
        const auto pullFromView = [vm, structProp, widget, setField]() {
            Struct s = vm->getProperty(structProp).value<Struct>();
            setField(s, widget->isChecked());
            vm->setProperty(structProp, QVariant::fromValue(s));
        };

        BindingInternal::connectStructVmToView(vm, structProp, widget, applyToView, session);
        BindingInternal::connectViewToVm(widget, vm, options, pullFromView);
        return BindingInternal::registerPullHandle(session, widget, pullFromView);
    }

    template<typename Struct>
    static BindingHandle *bindStructField(ViewModel *vm, const QString &structProp,
                                          std::function<QString(const Struct &)> getField,
                                          std::function<void(Struct &, const QString &)> setField,
                                          QComboBox *widget, const BindingOptions &options = {},
                                          BindingSession *session = nullptr)
    {
        if (!widget || !vm || !vm->hasProperty(structProp))
            return nullptr;

        const auto applyToView = [widget, getField](const QVariant &val) {
            QSignalBlocker blocker(widget);
            widget->setCurrentText(getField(val.value<Struct>()));
        };
        const auto pullFromView = [vm, structProp, widget, setField]() {
            Struct s = vm->getProperty(structProp).value<Struct>();
            setField(s, widget->currentText());
            vm->setProperty(structProp, QVariant::fromValue(s));
        };

        BindingInternal::connectStructVmToView(vm, structProp, widget, applyToView, session);
        BindingInternal::connectViewToVm(widget, vm, options, pullFromView);
        return BindingInternal::registerPullHandle(session, widget, pullFromView);
    }

    template<typename Struct>
    static BindingHandle *bindStructIndexField(ViewModel *vm, const QString &structProp,
                                               std::function<int(const Struct &)> getField,
                                               std::function<void(Struct &, int)> setField,
                                               QComboBox *widget, const BindingOptions &options = {},
                                               BindingSession *session = nullptr)
    {
        if (!widget || !vm || !vm->hasProperty(structProp))
            return nullptr;

        const auto applyToView = [widget, getField](const QVariant &val) {
            QSignalBlocker blocker(widget);
            widget->setCurrentIndex(getField(val.value<Struct>()));
        };
        const auto pullFromView = [vm, structProp, widget, setField]() {
            Struct s = vm->getProperty(structProp).value<Struct>();
            setField(s, widget->currentIndex());
            vm->setProperty(structProp, QVariant::fromValue(s));
        };

        BindingInternal::connectStructVmToView(vm, structProp, widget, applyToView, session);
        if (options.mode == BindingMode::TwoWay && options.trigger == UpdateSourceTrigger::OnChanged) {
            QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), vm,
                             [pullFromView](int) { pullFromView(); });
        }
        return BindingInternal::registerPullHandle(session, widget, pullFromView);
    }

    /**
     * @brief 绑定结构体字段（int 字段 -> QProgressBar，单向）
     */
    template<typename Struct>
    static void bindStructFieldOneWay(ViewModel *vm, const QString &structProp,
                                      std::function<int(const Struct &)> getField,
                                      QProgressBar *widget)
    {
        if (!widget || !vm || !vm->hasProperty(structProp)) return;

        widget->setValue(getField(vm->getProperty(structProp).value<Struct>()));
        QObject::connect(vm, &ViewModel::propertyChanged, widget,
            [widget, structProp, getField](const QString &name, const QVariant &val) {
                if (name == structProp)
                    widget->setValue(getField(val.value<Struct>()));
            });
    }

private:
    // 辅助函数：从ViewModel属性名生成信号名
    static QString getSignalName(const QString &propertyName);

    // 辅助函数：连接ViewModel信号到槽
    static void connectPropertyChanged(ViewModel *viewModel, const QString &propertyName,
                                      QObject *receiver, const char *slot);
};

#endif // DATABINDING_H
