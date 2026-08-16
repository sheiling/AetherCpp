#ifndef USERVIEWMODEL_H
#define USERVIEWMODEL_H

#include "viewmodel.h"
#include "command.h"
#include "usermodel.h"

/**
 * @brief 用户管理ViewModel（示例）
 * 
 * 展示MVVM框架的完整使用：
 * 1. 使用VIEWMODEL_PROPERTY宏定义属性
 * 2. 使用Command封装用户操作
 * 3. 业务逻辑处理
 * 4. 与Model层交互
 */
class UserViewModel : public ViewModel
{
    Q_OBJECT

public:
    // 属性名字面量，用于 DataBinding 避免硬编码字符串（拼写错误可编译期发现需配合常量使用）
    static constexpr const char* PROP_USER_NAME = "userName";
    static constexpr const char* PROP_USER_AGE = "userAge";
    static constexpr const char* PROP_USER_EMAIL = "userEmail";
    static constexpr const char* PROP_IS_ACTIVE = "isActive";
    static constexpr const char* PROP_STATUS_MESSAGE = "statusMessage";
    static constexpr const char* PROP_USER_COUNT = "userCount";

    // 使用宏定义属性（自动生成getter/setter）
    VIEWMODEL_PROPERTY(QString, userName, UserName)
    VIEWMODEL_PROPERTY(int, userAge, UserAge)
    VIEWMODEL_PROPERTY(QString, userEmail, UserEmail)
    VIEWMODEL_PROPERTY(bool, isActive, IsActive)
    VIEWMODEL_PROPERTY(QString, statusMessage, StatusMessage)
    VIEWMODEL_PROPERTY(int, userCount, UserCount)

signals:
    // 属性变更信号
    void userNameChanged();
    void userAgeChanged();
    void userEmailChanged();
    void isActiveChanged();
    void statusMessageChanged();
    void userCountChanged();

public:
    explicit UserViewModel(QObject *parent = nullptr);
    ~UserViewModel() override;

    // 获取Command
    Command* saveCommand() const { return m_saveCommand; }
    Command* clearCommand() const { return m_clearCommand; }
    Command* loadCommand() const { return m_loadCommand; }

public slots:
    // 业务逻辑方法
    void loadUsers();
    void saveUser();
    void clearForm();
    
private:
    void updateCommandStates();
    bool canSave() const;
    
private:
    // Commands
    FunctionalCommand *m_saveCommand;
    FunctionalCommand *m_clearCommand;
    FunctionalCommand *m_loadCommand;
};

#endif // USERVIEWMODEL_H
