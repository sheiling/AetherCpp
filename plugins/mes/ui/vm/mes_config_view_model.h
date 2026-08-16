#pragma once

#include "viewmodel.h"
#include "mes_config_model.h"

namespace mes {

/**
 * @brief MES 配置 ViewModel
 */
class MesConfigViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_CONFIG = "config";
    static constexpr const char *PROP_STATUS = "status";

    VIEWMODEL_STRUCT_PROPERTY(MesConfig, config, Config)
    VIEWMODEL_PROPERTY(QString, status, Status)

signals:
    void configChanged();
    void statusChanged();

public:
    explicit MesConfigViewModel(QObject *parent = nullptr);

    void loadConfig();
    void saveConfig();
};

} // namespace mes
