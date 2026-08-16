#pragma once

#include <QObject>

class QTimer;

namespace permission {

/// 空闲超时守卫：无操作超过写死时长后发出 idleTimeout（仅 sessionActive 时计时）
class SessionIdleGuard : public QObject
{
    Q_OBJECT

public:
    explicit SessionIdleGuard(QObject *parent = nullptr);

    void setIdleTimeoutMs(int ms);
    int  idleTimeoutMs() const { return m_idleTimeoutMs; }

    /// 安装到 QApplication，监听全局键鼠操作
    void installGlobal();

    /// 是否已登录会话；false 时停止计时
    void setSessionActive(bool active);

signals:
    void idleTimeout();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void resetIdleTimer();

    QTimer *m_timer           = nullptr;
    int     m_idleTimeoutMs   = 0;
    bool    m_sessionActive   = false;
    bool    m_installed       = false;
};

} // namespace permission
