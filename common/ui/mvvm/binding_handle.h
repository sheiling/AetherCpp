#ifndef BINDING_HANDLE_H
#define BINDING_HANDLE_H

/**
 * @file binding_handle.h
 * @brief 单条绑定的句柄，用于 Manual 模式下显式 flush（WPF UpdateSource）
 */

#include "mvvm_global.h"

#include <functional>

class QWidget;

/**
 * @brief 绑定句柄抽象：在 Manual / 保存前调用 updateSource() 将控件当前值写回 ViewModel
 */
class MVVM_EXPORT BindingHandle
{
public:
    virtual ~BindingHandle() = default;

    virtual void updateSource() = 0;
    virtual QWidget *boundWidget() const = 0;
};

/** 基于 std::function 的句柄实现，由 DataBinding 内部创建 */
class MVVM_EXPORT LambdaBindingHandle final : public BindingHandle
{
public:
    LambdaBindingHandle(QWidget *widget, std::function<void()> updateSourceFn);

    void updateSource() override;
    QWidget *boundWidget() const override;

private:
    QWidget               *m_widget = nullptr;
    std::function<void()> m_updateSource;
};

#endif // BINDING_HANDLE_H
