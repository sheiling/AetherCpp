#pragma once

#include "pagewidget.h"

#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

/**
 * @brief ContainerTypeView — Container 类型绑定演示页面
 *
 * 每个演示区块都创建一个独立的局部 Container::Container 实例，
 * 演示完毕后自动销毁，互不干扰。
 *
 * 演示要点：
 *  ① bind<Abstract, Concrete>()         — 瞬态绑定，每次 make() 返回全新实例
 *  ② singleton<Abstract, Concrete>()    — 单例绑定，所有 make() 返回同一实例
 *  ③ instance<Abstract>(preBuiltPtr)    — 注册预建实例，make() 直接返回此对象
 *  ④ extend<Abstract>(decorator)        — 装饰扩展，AOP 包装已有绑定
 *  ⑤ when<A>().needs<B>().give<C>()     — 上下文绑定，按调用者定制依赖
 *  ⑥ tag<A,B,...>(label) + tagged(label)— 标签分组，批量解析同类服务
 */
class ContainerTypeView : public PageWidget
{
    Q_OBJECT

public:
    explicit ContainerTypeView(QWidget *parent = nullptr);

protected:
    void onLoad()        override;
    void retranslateUi() override;

private:
    void setupUi();
    void setupConnections();

    // ── 演示区块控件 ─────────────────────────────────────────────────────
    QGroupBox   *m_bindGroup        = nullptr;
    QPushButton *m_bindBtn          = nullptr;

    QGroupBox   *m_singletonGroup   = nullptr;
    QPushButton *m_singletonBtn     = nullptr;

    QGroupBox   *m_instanceGroup    = nullptr;
    QPushButton *m_instanceBtn      = nullptr;

    QGroupBox   *m_extendGroup      = nullptr;
    QPushButton *m_extendBtn        = nullptr;

    QGroupBox   *m_contextualGroup  = nullptr;
    QPushButton *m_contextualBtn    = nullptr;

    QGroupBox   *m_tagGroup         = nullptr;
    QPushButton *m_tagBtn           = nullptr;

    // ── 调用记录 ──────────────────────────────────────────────────────────
    QGroupBox   *m_logGroup         = nullptr;
    QTextEdit   *m_logEdit          = nullptr;
    QPushButton *m_clearLogBtn      = nullptr;

    // ── 辅助 ──────────────────────────────────────────────────────────────
    void appendLog(const QString &line, const QString &color = "#d4d4d4");
    void appendSection(const QString &title);
};
