#include "mes_config_view_model.h"

#include <QMetaType>

namespace mes {

MesConfigViewModel::MesConfigViewModel(QObject *parent)
    : ViewModel(parent)
{
    qRegisterMetaType<MesConfig>("MesConfig");

    registerProperty(PROP_CONFIG, QVariant::fromValue(MesConfig{}));
    registerProperty(PROP_STATUS, QStringLiteral("Ready"));
}

void MesConfigViewModel::loadConfig()
{
    // TODO: 从持久化存储加载 MES 配置
    setStatus(QStringLiteral("Config loaded."));
}

void MesConfigViewModel::saveConfig()
{
    // TODO: 持久化 MES 配置
    setStatus(QStringLiteral("Config saved."));
}

} // namespace mes
