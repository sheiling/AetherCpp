#ifndef AUDITLOGMIDDLEWARE_H
#define AUDITLOGMIDDLEWARE_H

#include "imiddleware.h"
#include <QDateTime>
#include <QList>
#include <functional>

/**
 * @brief 审计日志中间件
 *
 * 记录每一次成功的属性变更，形成可追溯的变更历史。
 * 可通过 setOnEntryAdded 回调实时通知 UI 刷新历史列表。
 *
 * 不修改任何数据，始终调用 next()，仅在变更未被取消后记录。
 *
 * 使用方式：
 * @code
 * auto audit = std::make_shared<AuditLogMiddleware>();
 * audit->setOnEntryAdded([this](const AuditLogMiddleware::Entry &e) {
 *     m_histEdit->append(QString("[%1] %2: %3 → %4")
 *         .arg(e.timestamp.toString("HH:mm:ss"))
 *         .arg(e.propertyName)
 *         .arg(e.oldValue.toString())
 *         .arg(e.newValue.toString()));
 * });
 * viewModel->addPropertyMiddleware(audit);
 * @endcode
 */
class AuditLogMiddleware : public IPropertyMiddleware
{
public:
    /// 单条审计记录
    struct Entry {
        QDateTime timestamp;    ///< 变更时间
        QString   propertyName; ///< 属性名
        QVariant  oldValue;     ///< 变更前的值
        QVariant  newValue;     ///< 变更后的值（实际写入值）
    };

    using OnEntryAdded = std::function<void(const Entry &)>;

    /// 设置新增记录回调（可直接更新 UI）
    void setOnEntryAdded(OnEntryAdded cb) { m_onEntryAdded = std::move(cb); }

    /// 获取完整变更历史
    const QList<Entry> &history() const { return m_history; }

    /// 清空历史
    void clearHistory() { m_history.clear(); }

    void process(PropertyContext &ctx, Next next) override;

private:
    QList<Entry> m_history;
    OnEntryAdded m_onEntryAdded;
};

#endif // AUDITLOGMIDDLEWARE_H
