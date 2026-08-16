#pragma once

#include "pagewidget.h"
#include "container/application.h"

#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

/**
 * @brief ContainerNamedView — Container 命名绑定演示页面
 *
 * 演示命名绑定（Named Binding）的核心特性：
 * 调用方 **无需引入服务的具体头文件**，仅凭字符串名称即可完成
 * 服务解析、方法调用和属性读写。
 *
 * 演示要点：
 *  ① singletonNamed("logger", factory, setup) — 注册命名单例（含反射方法表）
 *  ② makeNamed("logger")                      — 解析为 ServiceHandle
 *  ③ handle.call("info", {any{msg}})           — 动态调用方法（无需类型信息）
 *  ④ handle.get("path") / handle.set("path")  — 属性读写（readWrite 注册）
 *  ⑤ boundNamed("logger") / hasMethod("info") — 存在性检查
 *  ⑥ singletonNamed("storage", ...) + call("store"/"retrieve") — 多参数调用
 *
 * 容器由 ContainerPlugin 在 bootstrap() 阶段注册好后以指针传入，
 * 本视图直接在已有的全局 Application 上操作，展示跨模块使用场景。
 */
class ContainerNamedView : public PageWidget
{
    Q_OBJECT

public:
    /**
     * @param app  ContainerPlugin 的 Application 实例（已 bootstrap）
     */
    explicit ContainerNamedView(Container::Application *app, QWidget *parent = nullptr);

protected:
    void onLoad()        override;
    void retranslateUi() override;

private:
    void setupUi();
    void setupConnections();

    // ── 成员 ──────────────────────────────────────────────────────────────
    Container::Application *m_app = nullptr;  ///< 插件的 Application 实例

    // ── Group 1: 注册状态 ─────────────────────────────────────────────────
    QGroupBox   *m_regGroup     = nullptr;
    QPushButton *m_checkRegBtn  = nullptr;

    // ── Group 2: call() 方法调用 ──────────────────────────────────────────
    QGroupBox   *m_callGroup    = nullptr;
    QLineEdit   *m_msgEdit      = nullptr;
    QPushButton *m_infoBtn      = nullptr;
    QPushButton *m_warnBtn      = nullptr;
    QPushButton *m_nameBtn      = nullptr;
    QPushButton *m_lastMsgBtn   = nullptr;

    // ── Group 3: 属性 get/set ─────────────────────────────────────────────
    QGroupBox   *m_propGroup    = nullptr;
    QLabel      *m_curPathLabel = nullptr;
    QLineEdit   *m_newPathEdit  = nullptr;
    QPushButton *m_getPathBtn   = nullptr;
    QPushButton *m_setPathBtn   = nullptr;

    // ── Group 4: storage 多参数调用 ───────────────────────────────────────
    QGroupBox   *m_storeGroup   = nullptr;
    QLineEdit   *m_keyEdit      = nullptr;
    QLineEdit   *m_valueEdit    = nullptr;
    QPushButton *m_storeBtn     = nullptr;
    QPushButton *m_retrieveBtn  = nullptr;
    QPushButton *m_hasBtn       = nullptr;
    QPushButton *m_countBtn     = nullptr;
    QPushButton *m_clearBtn     = nullptr;

    // ── Group 5: 存在性检查 ────────────────────────────────────────────────
    QGroupBox   *m_checkGroup   = nullptr;
    QPushButton *m_boundLoggerBtn   = nullptr;
    QPushButton *m_boundMissingBtn  = nullptr;
    QPushButton *m_hasMethodBtn     = nullptr;
    QPushButton *m_noMethodBtn      = nullptr;

    // ── 调用记录 ──────────────────────────────────────────────────────────
    QGroupBox   *m_logGroup     = nullptr;
    QTextEdit   *m_logEdit      = nullptr;
    QPushButton *m_clearLogBtn  = nullptr;

    void appendLog(const QString &line, const QString &color = "#d4d4d4");
    void appendSection(const QString &title);
};
