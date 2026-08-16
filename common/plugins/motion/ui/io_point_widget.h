#pragma once

#include <QFrame>

class QLabel;
class QHBoxLayout;
class QLineEdit;
class QRadioButton;
class QWidget;

namespace motion {

/** 单路 IO 配置公共基类 */
class IoPointWidget : public QFrame
{
    Q_OBJECT

public:
    static IoPointWidget *create(const QString &title, bool isOutput, QWidget *parent = nullptr);

    void applyFromConfig(int port, bool highActive);
    virtual void setActiveState(bool active) = 0;

    QString title() const { return m_title; }
    int     port() const;
    bool    highActive() const;

    /** true（默认）：显示全部表单；false：仅显示标题行（名称 + 状态指示） */
    void setShowFullDetails(bool show);
    bool showFullDetails() const { return m_showFullDetails; }

protected:
    explicit IoPointWidget(const QString &title, bool isOutput, QWidget *parent = nullptr);

    void initCommonUi();
    void insertStateWidget(QWidget *widget);
    void applyDetailsVisibility();
    void updatePortHeaderLabel(int port);

    QString m_title;
    bool    m_isOutput        = false;
    bool    m_showFullDetails = true;

    QHBoxLayout      *m_headLayout    = nullptr;
    QLabel           *m_portHeaderLbl = nullptr;
    QWidget          *m_detailSection = nullptr;
    QRadioButton     *m_highActive    = nullptr;
    QRadioButton     *m_lowActive     = nullptr;
    QLineEdit        *m_portEdit      = nullptr;
};

} // namespace motion
