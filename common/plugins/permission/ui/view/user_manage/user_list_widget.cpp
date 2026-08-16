#include "ui/view/user_manage/user_list_widget.h"

#include "uibase/permission/permservice.h"
#include "ui/view/permission_ui_utils.h"
#include "ui/view/user_manage/user_list_header_widget.h"
#include "ui/view/user_manage/user_row_delegate.h"

#include "theme/theme_refresh.h"

#include <QListView>
#include <QMetaType>
#include <QVBoxLayout>

namespace {

struct MetaTypeRegistration {
    MetaTypeRegistration() { qRegisterMetaType<UserRowAction>("UserRowAction"); }
};
static MetaTypeRegistration s_registerUserRowAction;

} // namespace

UserListWidget::UserListWidget(QWidget *parent)
    : QWidget(parent)
    , m_header(new UserListHeaderWidget(this))
    , m_list(new QListView(this))
    , m_model(new UserListItemModel(this))
    , m_delegate(new UserRowDelegate(this))
{
    setObjectName(QLatin1String(permission_ui::kObjUserListWrap));
    setAttribute(Qt::WA_StyledBackground, true);
    permission_ui::applyUserListWrapStyle(this);

    ui::bindThemeRefresh(this, [this]() {
        permission_ui::applyUserListWrapStyle(this);
        if (m_header)
            m_header->update();
        if (m_list && m_list->viewport())
            m_list->viewport()->update();
    });

    permission_ui::initUserList(m_list);
    m_list->setModel(m_model);
    m_list->setItemDelegate(m_delegate);
    m_list->setFrameShape(QFrame::NoFrame);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setMouseTracking(true);

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(m_header);
    lay->addWidget(m_list, 1);

    connect(m_model, &UserListItemModel::selectionChanged, this, &UserListWidget::selectionChanged);

    connect(m_header, &UserListHeaderWidget::checkToggled, this, [this](bool checked) {
        if (!m_model)
            return;
        const Qt::CheckState target = checked ? Qt::Checked : Qt::Unchecked;
        const int total = m_model->rowCount();
        for (int i = 0; i < total; ++i)
            m_model->setData(m_model->index(i, 0), target, Qt::CheckStateRole);
    });

    connect(m_delegate, &UserRowDelegate::rowActionTriggered, this,
            [this](UserRowAction action, qint64 userId) {
                switch (action) {
                case UserRowAction::EditUser:
                    emit editRequested(userId);
                    break;
                case UserRowAction::ResetPassword:
                    emit resetPasswordRequested(userId);
                    break;
                case UserRowAction::ToggleEnabled: {
                    for (const auto &row : m_model->rows()) {
                        if (row.id == userId) {
                            emit toggleStatusRequested(userId, row.accountEnabled);
                            break;
                        }
                    }
                    break;
                }
                }
            });

    connect(PS, &PermService::permissionsChanged, m_list, [this]() {
        if (m_list)
            m_list->viewport()->update();
    });
}

void UserListWidget::setColumnHeaders(const QStringList &headers)
{
    m_header->setHeaders(headers);
}

void UserListWidget::setActionLabels(const QString &edit, const QString &resetPassword,
                                     const QString &enable, const QString &disable)
{
    m_delegate->setActionLabels(edit, resetPassword, enable, disable);
}

void UserListWidget::setRows(const QVector<UserListDisplayRow> &rows)
{
    m_model->setRows(rows);
    m_header->setChecked(false);
}

QVector<qint64> UserListWidget::selectedUserIds() const
{
    return m_model->selectedUserIds();
}
