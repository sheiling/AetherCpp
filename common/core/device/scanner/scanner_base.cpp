#include "scanner_base.h"

#include <common/core/messagecenter/message_center.h>

#include <QString>

namespace common
{
namespace device
{

void ScannerBase::SetConnectedState(bool connected)
{
    if (isConnected_ == connected) {
        return;
    }
    isConnected_ = connected;

    const QString name = m_deviceName.empty()
        ? QString::fromStdString(m_scannerDevInfo_.ip.empty()
            ? m_scannerDevInfo_.serialInfo.com_
            : m_scannerDevInfo_.ip)
        : QString::fromStdString(m_deviceName);
    MessageCenter::Center::Notify(EVENT_SCANNER_CONN_STATE, name, connected);
}

} // namespace device
} // namespace common
