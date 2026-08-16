#ifndef COMMAND_H
#define COMMAND_H

#include "mvvm_global.h"
#include "middlewarepipeline.h"
#include <QObject>
#include <functional>

// 前向声明
class ViewModel;

/**
 * @brief Command基类，用于封装用户操作
 * 
 * Command模式将用户操作封装为对象，支持：
 * - 执行/撤销操作
 * - 可用性控制
 * - 信号通知
 */
class MVVM_EXPORT Command : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canExecute READ canExecute NOTIFY canExecuteChanged)

public:
    explicit Command(QObject *parent = nullptr);
    virtual ~Command() = default;

    // 执行命令（直接执行，跳过中间件）
    virtual void execute() = 0;

    // 命令是否可执行
    virtual bool canExecute() const { return m_canExecute; }

    // 设置可执行状态
    void setCanExecute(bool can);

    // ── 命令名称（中间件日志；非空时亦作为 perm_permission.key 参与 canExecute）──
    void setName(const QString &name);
    QString name() const;

    // ── 关联 ViewModel（可选，中间件可通过 CommandContext::viewModel 访问）─────
    void setViewModel(ViewModel *vm);
    ViewModel *viewModel() const;

    // ── 命令中间件管道 ───────────────────────────────────────────────────────
    /**
     * @brief 向管道末尾追加命令中间件
     *
     * 示例：
     * @code
     * command->addMiddleware(std::make_shared<AuthMiddleware>());
     * command->addMiddleware(std::make_shared<LoggingCommandMiddleware>());
     * @endcode
     */
    void addMiddleware(CommandMiddlewarePtr middleware);

    /// 清空所有命令中间件
    void clearMiddleware();

public slots:
    /**
     * @brief 经中间件管道后执行命令（推荐在 UI 绑定中使用此方法）
     *
     * 流程：canExecute 检查 → 运行中间件 → 若未取消则调用 execute()
     * DataBinding::bindCommand 自动连接按钮到此 slot。
     */
    void run();

signals:
    void canExecuteChanged(bool canExecute);

protected:
    bool                  m_canExecute  = true;

private:
    QString               m_name;
    ViewModel            *m_viewModel   = nullptr;
    CommandMiddlewarePipeline m_cmdPipeline;
};

/**
 * @brief 函数式Command，使用lambda或函数对象
 * 
 * 使用示例：
 * auto cmd = new FunctionalCommand([this]() {
 *     qDebug() << "Command executed!";
 * });
 */
class MVVM_EXPORT FunctionalCommand : public Command
{
    Q_OBJECT

public:
    using ExecuteFunction = std::function<void()>;
    using CanExecuteFunction = std::function<bool()>;

    explicit FunctionalCommand(ExecuteFunction executeFunc, 
                               CanExecuteFunction canExecuteFunc = nullptr,
                               QObject *parent = nullptr);

    void execute() override;
    bool canExecute() const override;

    // 更新可执行状态（会发出信号）
    void updateCanExecute();

private:
    ExecuteFunction m_executeFunc;
    CanExecuteFunction m_canExecuteFunc;
};

#endif // COMMAND_H
