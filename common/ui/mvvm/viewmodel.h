#ifndef VIEWMODEL_H
#define VIEWMODEL_H

#include "mvvm_global.h"
#include "middlewarepipeline.h"
#include <QObject>
#include <QVariant>
#include <QHash>
#include <functional>

/**
 * @brief ViewModel基类
 *
 * MVVM模式的ViewModel层，负责：
 * 1. 管理UI状态数据
 * 2. 提供属性变更通知
 * 3. 封装业务逻辑
 * 4. 不包含任何UI代码
 *
 * 使用示例：
 * class MyViewModel : public ViewModel {
 *     Q_OBJECT
 * public:
 *     VIEWMODEL_PROPERTY(QString, userName, UserName)
 *     VIEWMODEL_PROPERTY(int, age, Age)
 * };
 */
class MVVM_EXPORT ViewModel : public QObject
{
    Q_OBJECT

public:
    explicit ViewModel(QObject *parent = nullptr);
    virtual ~ViewModel() = default;

    // 获取/设置属性值
    QVariant getProperty(const QString &propertyName) const;
    void setProperty(const QString &propertyName, const QVariant &value);

    // 安全获取：属性存在时写入 out 并返回 true，否则返回 false
    bool getProperty(const QString &propertyName, QVariant &out) const;

    // 检查属性是否存在
    bool hasProperty(const QString &propertyName) const;

    // 批量更新：在 begin/end 之间多次 setProperty 只会在 endBatchUpdate 时统一发出信号
    void beginBatchUpdate();
    void endBatchUpdate();
    bool isBatchUpdating() const { return m_inBatchUpdate; }

    // ── Middleware 支持 ────────────────────────────────────────────────────────
    /**
     * @brief 向属性变更管道末尾追加一个中间件
     *
     * 中间件按注册顺序依次调用，可拦截/修改/取消属性变更。
     *
     * @code
     * viewModel->addPropertyMiddleware(std::make_shared<LoggingMiddleware>());
     * viewModel->addPropertyMiddleware(std::make_shared<ValidationMiddleware>());
     * @endcode
     */
    void addPropertyMiddleware(PropertyMiddlewarePtr middleware);

    /// 清空所有属性中间件
    void clearPropertyMiddleware();

    /// 当前已注册的属性中间件数量
    int propertyMiddlewareCount() const;

signals:
    // 属性值变更信号（通用）
    void propertyChanged(const QString &propertyName, const QVariant &value);

protected:
    // 注册属性（子类在构造函数中调用）
    void registerProperty(const QString &propertyName, const QVariant &defaultValue = QVariant());

    // 通知属性变更
    void notifyPropertyChanged(const QString &propertyName);

    /** 无条件写入属性（跳过 QVariant 相等判断，用于 struct 整包提交） */
    void assignProperty(const QString &propertyName, const QVariant &value);

private:
    void endBatchUpdateImpl();

    QHash<QString, QVariant>   m_properties;                // O(1) 查找
    bool                       m_inBatchUpdate = false;
    QHash<QString, QVariant>   m_pendingChanges;            // 批量更新时暂存变更
    PropertyMiddlewarePipeline m_propertyPipeline;          // 属性中间件管道
};

// ============================================================================
// 便捷宏：定义基础类型 ViewModel 属性
// ============================================================================

/**
 * @brief 便捷宏：定义基础类型 ViewModel 属性
 *
 * 自动生成 getter / setter，setter 在值变化时发出 propertyChanged 信号
 * 以及对应的具名变更信号（需在 signals: 块中声明）。
 *
 * 使用示例：
 *   VIEWMODEL_PROPERTY(QString, userName, UserName)
 *   // 需同时在 signals: 中声明：void userNameChanged();
 *
 * 建议用常量替代硬编码字符串：
 *   static constexpr const char* PROP_UserName = "userName";
 */
#define VIEWMODEL_PROPERTY(Type, name, Name) \
public: \
    Type name() const { \
        return getProperty(#name).value<Type>(); \
    } \
    void set##Name(const Type &value) { \
        if (name() != value) { \
            setProperty(#name, QVariant::fromValue(value)); \
            emit name##Changed(); \
            emit propertyChanged(#name, QVariant::fromValue(value)); \
        } \
    } \
private:

// ============================================================================
// 便捷宏：定义结构体类型 ViewModel 属性（需 Q_DECLARE_METATYPE 注册）
// ============================================================================

/**
 * @brief 便捷宏：定义结构体类型 ViewModel 属性
 *
 * 适用于需要整体存储的自定义结构体（如配置信息、设备描述等）。
 * 结构体类型需在头文件末尾使用 Q_DECLARE_METATYPE(Type) 注册，
 * 并在 ViewModel 构造函数中调用 registerProperty(#name, QVariant::fromValue(Type{}))。
 *
 * 生成成员：
 *   - Type  name()          const   // getter：返回完整结构体副本
 *   - void  setName(const Type&)    // setter：整体替换并发出信号
 *   - void  modifyName(std::function<void(Type&)>)  // 字段级修改器
 *
 * 字段级 UI 绑定示例（配合 DataBinding::bindStructField<T>）：
 * @code
 *   DataBinding::bindStructField<DeviceInfo>(vm, "device",
 *       [](const DeviceInfo &d){ return d.name; },
 *       [](DeviceInfo &d, const QString &v){ d.name = v; }, nameEdit);
 * @endcode
 *
 * 注意：由于 QVariant 对自定义类型的相等比较依赖 QMetaType::registerComparators<T>()，
 *       如需精确去重（避免值未变时重复发信号），请在应用初始化时注册比较器：
 *         QMetaType::registerComparators<DeviceInfo>();
 *       若未注册，setter 每次调用都会触发信号（功能正确，仅有微小性能开销）。
 */
#define VIEWMODEL_STRUCT_PROPERTY(Type, name, Name) \
public: \
    Type name() const { \
        return getProperty(#name).value<Type>(); \
    } \
    void set##Name(const Type &value) { \
        setProperty(#name, QVariant::fromValue(value)); \
        emit name##Changed(); \
    } \
    void modify##Name(std::function<void(Type &)> modifier) { \
        Type current = name(); \
        modifier(current); \
        set##Name(current); \
    } \
private:

#endif // VIEWMODEL_H
