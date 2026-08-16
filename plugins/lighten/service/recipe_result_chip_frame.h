#pragma once

/**
 * @file service/recipe_result_chip_frame.h
 * @brief 配方调试区结果输出单项芯片（整块可点，含子控件区域）
 */

#include "lighten_types.h"

#include <QFrame>
#include <QMouseEvent>

class QWidget;

namespace lighten {

/** 结果输出区单项芯片：子控件透传鼠标事件，整块区域点击发出 sig_clicked(DetectItemKind) */
class RecipeResultChipFrame final : public QFrame
{
    Q_OBJECT
public:
    explicit RecipeResultChipFrame(DetectItemKind kind, QWidget *parent = nullptr);

    DetectItemKind itemKind() const { return m_kind; }

    /** 子 QLabel 等不拦截点击，统一由本 QFrame 接收 */
    static void makeChildrenMouseTransparent(QWidget *root);

signals:
    void sig_clicked(DetectItemKind kind);

protected:
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    DetectItemKind m_kind;
};

} // namespace lighten
