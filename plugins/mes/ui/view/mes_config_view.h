#pragma once

#include "pagewidget.h"

namespace mes {

class MesConfigViewModel;

/**
 * @brief MES 配置页面
 */
class MesConfigView : public PageWidget
{
    Q_OBJECT

public:
    explicit MesConfigView(QWidget *parent = nullptr);
    ~MesConfigView() override;

protected:
    void onLoad() override;

private:
    void buildLayout();

    MesConfigViewModel *m_viewModel = nullptr;
};

} // namespace mes
