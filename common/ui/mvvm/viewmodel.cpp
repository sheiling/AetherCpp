#include "viewmodel.h"

ViewModel::ViewModel(QObject *parent)
    : QObject(parent)
{
}

void ViewModel::registerProperty(const QString &propertyName, const QVariant &defaultValue)
{
    m_properties[propertyName] = defaultValue;
}

QVariant ViewModel::getProperty(const QString &propertyName) const
{
    return m_properties.value(propertyName);
}

bool ViewModel::getProperty(const QString &propertyName, QVariant &out) const
{
    auto it = m_properties.constFind(propertyName);
    if (it != m_properties.constEnd()) {
        out = it.value();
        return true;
    }
    return false;
}

void ViewModel::setProperty(const QString &propertyName, const QVariant &value)
{
    auto it = m_properties.find(propertyName);
    if (it == m_properties.end())
        return;
    if (it.value() == value)
        return;

    if (m_propertyPipeline.isEmpty()) {
        // ── 快速路径：无中间件，直接写入 ──────────────────────────────────
        it.value() = value;
        if (m_inBatchUpdate) {
            m_pendingChanges[propertyName] = value;
        } else {
            emit propertyChanged(propertyName, value);
        }
    } else {
        // ── 中间件 around 路径：属性写入作为管道终端，嵌在链条最深处 ────────
        // 执行顺序：MW[0].pre → MW[1].pre → 写入属性 → MW[1].post → MW[0].post
        PropertyContext ctx(propertyName, it.value(), value, this);
        m_propertyPipeline.run(ctx, [this, &it](PropertyContext &c) {
            if (!c.cancelled) {
                it.value() = c.newValue;   // 使用中间件可能修改过的新值
                if (m_inBatchUpdate) {
                    m_pendingChanges[c.propertyName] = c.newValue;
                } else {
                    emit propertyChanged(c.propertyName, c.newValue);
                }
            }
        });
    }
}

void ViewModel::addPropertyMiddleware(PropertyMiddlewarePtr middleware)
{
    m_propertyPipeline.add(std::move(middleware));
}

void ViewModel::clearPropertyMiddleware()
{
    m_propertyPipeline.clear();
}

int ViewModel::propertyMiddlewareCount() const
{
    return m_propertyPipeline.count();
}

void ViewModel::beginBatchUpdate()
{
    m_inBatchUpdate = true;
    m_pendingChanges.clear();
}

void ViewModel::endBatchUpdate()
{
    endBatchUpdateImpl();
}

void ViewModel::endBatchUpdateImpl()
{
    if (!m_inBatchUpdate)
        return;
    m_inBatchUpdate = false;
    for (auto it = m_pendingChanges.constBegin(); it != m_pendingChanges.constEnd(); ++it)
        emit propertyChanged(it.key(), it.value());
    m_pendingChanges.clear();
}

bool ViewModel::hasProperty(const QString &propertyName) const
{
    return m_properties.contains(propertyName);
}

void ViewModel::notifyPropertyChanged(const QString &propertyName)
{
    if (m_properties.contains(propertyName)) {
        emit propertyChanged(propertyName, m_properties[propertyName]);
    }
}

void ViewModel::assignProperty(const QString &propertyName, const QVariant &value)
{
    auto it = m_properties.find(propertyName);
    if (it == m_properties.end())
        return;

    it.value() = value;
    if (m_inBatchUpdate) {
        m_pendingChanges[propertyName] = value;
    } else {
        emit propertyChanged(propertyName, value);
    }
}
