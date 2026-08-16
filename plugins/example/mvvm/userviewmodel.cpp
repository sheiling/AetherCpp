#include "userviewmodel.h"
#include <QDebug>

UserViewModel::UserViewModel(QObject *parent)
    : ViewModel(parent)
{
    // 注册属性（必须在使用VIEWMODEL_PROPERTY之前注册，使用常量保证与 PROP_* 一致）
    registerProperty(PROP_USER_NAME, QString());
    registerProperty(PROP_USER_AGE, 0);
    registerProperty(PROP_USER_EMAIL, QString());
    registerProperty(PROP_IS_ACTIVE, true);
    registerProperty(PROP_STATUS_MESSAGE, QString::fromUtf8("\xE5\xB0\xB1\xE7\xBB\xAA"));
    registerProperty(PROP_USER_COUNT, 0);
    
    // 创建Commands
    m_saveCommand = new FunctionalCommand(
        [this]() { saveUser(); },
        [this]() { return canSave(); },
        this
    );
    
    m_clearCommand = new FunctionalCommand(
        [this]() { clearForm(); },
        nullptr,
        this
    );
    
    m_loadCommand = new FunctionalCommand(
        [this]() { loadUsers(); },
        nullptr,
        this
    );
    
    // 监听属性变化，更新Command状态
    connect(this, &ViewModel::propertyChanged, this, [this]() {
        updateCommandStates();
    });
    
    // 初始加载
    loadUsers();
}

UserViewModel::~UserViewModel()
{
}

void UserViewModel::loadUsers()
{
    QList<User> users = UserRepository::getAllUsers();
    beginBatchUpdate();
    setUserCount(users.size());
    if (!users.isEmpty()) {
        const User &user = users.first();
        setUserName(user.name);
        setUserAge(user.age);
        setUserEmail(user.email);
        setIsActive(user.isActive);
        QString msg = QString::fromUtf8("\xE5\xB7\xB2\xE5\x8A\xA0\xE8\xBD\xBD %1 \xE4\xB8\xAA\xE7\x94\xA8\xE6\x88\xB7");
        setStatusMessage(msg.arg(users.size()));
    } else {
        setStatusMessage(QString::fromUtf8("\xE6\xB2\xA1\xE6\x9C\x89\xE7\x94\xA8\xE6\x88\xB7\xE6\x95\xB0\xE6\x8D\xAE"));
    }
    endBatchUpdate();
}

void UserViewModel::saveUser()
{
    // 创建User对象
    User user;
    user.name = userName();
    user.age = userAge();
    user.email = userEmail();
    user.isActive = isActive();
    
    // 保存到仓储
    if (UserRepository::saveUser(user)) {
        QString msg = QString::fromUtf8("\xE7\x94\xA8\xE6\x88\xB7 '%1' \xE4\xBF\x9D\xE5\xAD\x98\xE6\x88\x90\xE5\x8A\x9F\xEF\xBC\x81");
        setStatusMessage(msg.arg(user.name));
        clearForm();
        loadUsers(); // 重新加载
    } else {
        setStatusMessage(QString::fromUtf8("\xE4\xBF\x9D\xE5\xAD\x98\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x81"));
    }
}

void UserViewModel::clearForm()
{
    beginBatchUpdate();
    setUserName("");
    setUserAge(0);
    setUserEmail("");
    setIsActive(true);
    setStatusMessage(QString::fromUtf8("\xE8\xA1\xA8\xE5\x8D\x95\xE5\xB7\xB2\xE6\xB8\x85\xE7\xA9\xBA"));
    endBatchUpdate();
}

void UserViewModel::updateCommandStates()
{
    m_saveCommand->updateCanExecute();
}

bool UserViewModel::canSave() const
{
    // 验证：名字和邮箱不能为空，年龄必须大于0
    return !userName().isEmpty() 
        && !userEmail().isEmpty() 
        && userAge() > 0;
}
