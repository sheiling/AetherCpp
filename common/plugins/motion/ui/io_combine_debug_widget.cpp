#include "io_combine_debug_widget.h"

#include "motion_control_helper.h"

#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace motion {

IoCombineDebugWidget::IoCombineDebugWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *rootLay = new QVBoxLayout(this);
    rootLay->setContentsMargins(0, 0, 0, 0);
    rootLay->setSpacing(0);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *content   = new QWidget(scroll);
    auto *groupsLay = new QVBoxLayout(content);
    groupsLay->setContentsMargins(0, 0, 4, 0);
    groupsLay->setSpacing(12);

    const QVector<IoCombineGroup> groups = loadIoCombineConfig();
    if (groups.isEmpty()) {
        auto *hint = new QLabel(QStringLiteral("未找到有效的 ioCombine.json 配置"), content);
        hint->setObjectName(QStringLiteral("motionIoCombineEmptyHint"));
        hint->setAlignment(Qt::AlignCenter);
        groupsLay->addWidget(hint);
    } else {
        for (const IoCombineGroup &group : groups) {
            auto *box = new QGroupBox(group.group, content);
            box->setObjectName(QStringLiteral("motionIoCombineGroup"));
            auto *actionLay = new QHBoxLayout(box);
            actionLay->setContentsMargins(12, 16, 12, 12);
            actionLay->setSpacing(8);

            for (const IoCombineAction &action : group.actions) {
                auto *btn = new QPushButton(action.name, box);
                btn->setObjectName(QStringLiteral("motionIoCombineActionBtn"));
                btn->setCursor(Qt::PointingHandCursor);
                const QVector<IoCombineOutput> outputs = action.outputs;
                connect(btn, &QPushButton::clicked, this, [outputs]() {
                    writeIoCombineOutputs(outputs, "[IoCombineDebugWidget]");
                });
                actionLay->addWidget(btn);
            }
            actionLay->addStretch();

            groupsLay->addWidget(box);
        }
    }

    groupsLay->addStretch();
    scroll->setWidget(content);
    rootLay->addWidget(scroll);
}

} // namespace motion
