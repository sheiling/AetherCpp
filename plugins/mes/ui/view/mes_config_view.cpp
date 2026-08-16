#include "mes_config_view.h"

#include "ui/vm/mes_config_view_model.h"

#include <QFont>
#include <QLabel>
#include <QVBoxLayout>

namespace mes {

MesConfigView::MesConfigView(QWidget *parent)
    : PageWidget(parent)
    , m_viewModel(new MesConfigViewModel(this))
{
}

MesConfigView::~MesConfigView() = default;

void MesConfigView::onLoad()
{
    buildLayout();
    m_viewModel->loadConfig();
}

void MesConfigView::buildLayout()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    auto *title = new QLabel(QStringLiteral("MES Configuration"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto *hint = new QLabel(
        QStringLiteral("MES plugin structure is ready. "
                       "Configuration UI and HTTP integration will be added next."),
        this);
    hint->setWordWrap(true);

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addStretch();
}

} // namespace mes
