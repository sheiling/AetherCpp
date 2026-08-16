#pragma once

/**
 * @file aoi_param_binding.h
 * @brief AOI 自定义控件与 DataBinding 的桥接
 *
 * `PillSwitch` 无标准 Qt value 属性，本头提供 `bindStructFieldPillSwitch`，
 * 将 `toggled` 与 ViewModel struct 字段 getter/setter 对接，策略与
 * `DataBinding::bindStructField` 一致（VM→View 随 struct 属性变化；TwoWay+OnChanged 时 toggled 回写）。
 *
 * 典型用法：绑定 `SystemConfigViewModel::config` 或 `RecipeConfigViewModel::document`
 * 内 bool 字段到 PillSwitch；保存前通过 BindingSession 的 pull handle 执行 flush。
 *
 * @param vm 目标 ViewModel，须已 registerProperty(structProp)
 * @param structProp struct 属性名（如 `config` / `document`）
 * @param getField 从 struct 读 bool 到控件 checked 状态
 * @param setField 从控件 checked 写回 struct 字段
 * @param widget 目标 PillSwitch
 * @param options 绑定模式与触发策略，默认 TwoWay + OnChanged
 * @param session 可选 BindingSession，用于 registerPullHandle 统一 flush
 * @return 成功时返回 pull handle，失败返回 nullptr
 */

#include "binding_internal.h"
#include "binding_options.h"
#include "binding_session.h"
#include "databinding.h"
#include "viewmodel.h"

#include "component/controls/switch/pill_switch.h"

#include <QSignalBlocker>

namespace aoi {

template<typename Struct>
BindingHandle *bindStructFieldPillSwitch(ViewModel *vm, const QString &structProp,
                                         std::function<bool(const Struct &)> getField,
                                         std::function<void(Struct &, bool)> setField,
                                         ui::PillSwitch *widget,
                                         const BindingOptions &options = {},
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
    if (options.mode == BindingMode::TwoWay
        && options.trigger == UpdateSourceTrigger::OnChanged) {
        QObject::connect(widget, &ui::PillSwitch::toggled, vm,
                         [pullFromView](bool) { pullFromView(); });
    }
    return BindingInternal::registerPullHandle(session, widget, pullFromView);
}

} // namespace aoi
