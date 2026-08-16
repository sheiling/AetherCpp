#include "session/session_idle_guard.h"

#include <QApplication>
#include <QEvent>
#include <QTimer>

namespace permission {

namespace {

bool isUserActivityEvent(QEvent::Type type)
{
    switch (type) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::Wheel:
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
        return true;
    default:
        return false;
    }
}

} // namespace

SessionIdleGuard::SessionIdleGuard(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &SessionIdleGuard::idleTimeout);
}

void SessionIdleGuard::setIdleTimeoutMs(int ms)
{
    m_idleTimeoutMs = ms > 0 ? ms : 1;
    if (m_timer->isActive())
        resetIdleTimer();
}

void SessionIdleGuard::installGlobal()
{
    if (m_installed || !qApp)
        return;
    qApp->installEventFilter(this);
    m_installed = true;
}

void SessionIdleGuard::setSessionActive(bool active)
{
    if (m_sessionActive == active)
        return;
    m_sessionActive = active;
    if (m_sessionActive)
        resetIdleTimer();
    else
        m_timer->stop();
}

void SessionIdleGuard::resetIdleTimer()
{
    if (!m_sessionActive || m_idleTimeoutMs <= 0)
        return;
    m_timer->start(m_idleTimeoutMs);
}

bool SessionIdleGuard::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);
    if (!m_sessionActive || !event || !isUserActivityEvent(event->type()))
        return false;
    resetIdleTimer();
    return false;
}

} // namespace permission
