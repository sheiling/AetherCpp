#ifndef BINDING_OPTIONS_H
#define BINDING_OPTIONS_H

/**
 * @file binding_options.h
 * @brief WPF 风格绑定策略：方向（BindingMode）与写回时机（UpdateSourceTrigger）
 *
 * 对照关系：
 *   BindingMode::TwoWay     ↔ WPF BindingMode.TwoWay
 *   BindingMode::OneWay     ↔ WPF BindingMode.OneWay（仅 ViewModel → View）
 *   UpdateSourceTrigger::OnChanged   ↔ PropertyChanged
 *   UpdateSourceTrigger::OnLostFocus ↔ LostFocus
 *   UpdateSourceTrigger::Manual      ↔ Explicit（需 BindingSession::updateAllSources）
 *
 * VM → View 始终随 propertyChanged 即时刷新（与 UpdateSourceTrigger 无关）。
 */

#include "mvvm_global.h"

/** 绑定方向 */
enum class BindingMode
{
    TwoWay, /**< ViewModel ↔ View */
    OneWay  /**< 仅 ViewModel → View，控件编辑不写回 ViewModel */
};

/** View → ViewModel 的写回时机（仅 TwoWay 有效） */
enum class UpdateSourceTrigger
{
    OnChanged,   /**< 值变更时立即写回 */
    OnLostFocus, /**< 控件 editingFinished / 失焦时写回 */
    Manual       /**< 仅 BindingHandle::updateSource 或 BindingSession::updateAllSources */
};

/** 单条绑定的策略；默认双向 + 即时写回 */
struct MVVM_EXPORT BindingOptions
{
    BindingMode         mode    = BindingMode::TwoWay;
    UpdateSourceTrigger trigger = UpdateSourceTrigger::OnChanged;
};

#endif // BINDING_OPTIONS_H
