#pragma once

#include "ui/view/user_manage/user_list_layout.h"

#include <QStringList>
#include <QWidget>

class QMouseEvent;
class QPaintEvent;
class QPainter;

class UserListHeaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserListHeaderWidget(QWidget *parent = nullptr);

    void setHeaders(const QStringList &headers);
    void setChecked(bool checked);

signals:
    void checkToggled(bool checked);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void paintCheckbox(QPainter *painter, const QRect &rect, bool checked) const;

    QStringList m_headers;
    bool        m_checked = false;
};
