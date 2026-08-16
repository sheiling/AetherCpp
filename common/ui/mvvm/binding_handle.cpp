#include "binding_handle.h"

#include <QWidget>

// LambdaBindingHandle：将 pullFromView 闭包封装为可显式调用的 BindingHandle

LambdaBindingHandle::LambdaBindingHandle(QWidget *widget, std::function<void()> updateSourceFn)
    : m_widget(widget)
    , m_updateSource(std::move(updateSourceFn))
{
}

void LambdaBindingHandle::updateSource()
{
    if (m_updateSource)
        m_updateSource();
}

QWidget *LambdaBindingHandle::boundWidget() const
{
    return m_widget;
}
