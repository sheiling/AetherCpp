#ifndef BINDING_SESSION_H
#define BINDING_SESSION_H

/**
 * @file binding_session.h
 * @brief 页面级绑定会话：集中管理 Manual flush 与批量加载时的 VM→View 暂停
 *
 * 典型用法（View 成员 `BindingSession m_bindings`）：
 * @code
 *   DataBinding::bind(vm, "count", spin, {}, &m_bindings);
 *   // 保存前 flush Manual / LostFocus 未提交项
 *   m_bindings.updateAllSources();
 *   vm->save();
 *   // 切换配方 / 刷新列表时避免回写风暴
 *   m_bindings.beginSuspendUpdates();
 *   vm->setDocument(doc);
 *   m_bindings.endSuspendUpdates();
 * @endcode
 */

#include "binding_handle.h"
#include "mvvm_global.h"

#include <memory>
#include <vector>

class MVVM_EXPORT BindingSession
{
public:
    BindingSession() = default;
    BindingSession(const BindingSession &)            = delete;
    BindingSession &operator=(const BindingSession &) = delete;

    /** 登记绑定句柄；返回原始指针供调用方保存（可选） */
    BindingHandle *add(std::unique_ptr<BindingHandle> handle);

    /** 对所有已登记绑定执行 updateSource（Manual 保存、切 Tab 前 commit） */
    void updateAllSources();

    /** 嵌套计数：暂停期间 propertyChanged 不推送到控件 */
    void beginSuspendUpdates();
    void endSuspendUpdates();
    bool isSuspended() const { return m_suspendCount > 0; }

private:
    std::vector<std::unique_ptr<BindingHandle>> m_handles;
    int                                         m_suspendCount = 0;
};

#endif // BINDING_SESSION_H
