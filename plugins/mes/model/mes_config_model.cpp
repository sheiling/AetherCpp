#include "mes_config_model.h"

namespace mes {

bool MesConfig::operator==(const MesConfig &o) const
{
    return enabled == o.enabled
        && address == o.address
        && equipmentCode == o.equipmentCode
        && equipmentName == o.equipmentName
        && operatorId == o.operatorId
        && operatorName == o.operatorName
        && appKey == o.appKey
        && useCode == o.useCode
        && verType == o.verType
        && verNumber == o.verNumber
        && timeout == o.timeout
        && requestTimes == o.requestTimes;
}

} // namespace mes
