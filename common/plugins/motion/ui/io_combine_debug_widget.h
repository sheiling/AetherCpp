#pragma once

#include <QWidget>

namespace motion {

/** 联动调试：解析 config/ioCombine.json，按 group 分组展示动作按钮 */
class IoCombineDebugWidget : public QWidget
{
public:
    explicit IoCombineDebugWidget(QWidget *parent = nullptr);
};

} // namespace motion
