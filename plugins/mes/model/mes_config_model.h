#pragma once

#include <QMetaType>
#include <QString>

namespace mes {

/** MES 连接与设备配置 */
struct MesConfig
{
    bool    enabled      = false;
    QString address;
    QString equipmentCode;
    QString equipmentName;
    QString operatorId;
    QString operatorName;
    QString appKey;
    QString useCode;
    QString verType;
    QString verNumber;
    int     timeout      = 5;
    int     requestTimes = 5;

    bool operator==(const MesConfig &o) const;
    bool operator!=(const MesConfig &o) const { return !(*this == o); }
};

} // namespace mes

Q_DECLARE_METATYPE(mes::MesConfig)
