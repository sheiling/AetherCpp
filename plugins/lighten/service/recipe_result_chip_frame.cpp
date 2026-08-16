#include "recipe_result_chip_frame.h"

#include <QWidget>

namespace lighten {

RecipeResultChipFrame::RecipeResultChipFrame(DetectItemKind kind, QWidget *parent)
    : QFrame(parent)
    , m_kind(kind)
{
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

void RecipeResultChipFrame::makeChildrenMouseTransparent(QWidget *root)
{
    for (QWidget *w : root->findChildren<QWidget *>())
        w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void RecipeResultChipFrame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && rect().contains(e->pos()))
        emit sig_clicked(m_kind);
    QFrame::mouseReleaseEvent(e);
}

} // namespace lighten
