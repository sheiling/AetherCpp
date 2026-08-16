#ifndef USERVIEW_H
#define USERVIEW_H

#include "pagewidget.h"
#include "userviewmodel.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QFormLayout>

/**
 * @brief 用户管理视图（View层）- MVVM示例
 * 
 * 展示如何使用DataBinding绑定ViewModel：
 * 1. 不在View中编写业务逻辑
 * 2. 使用DataBinding进行数据绑定
 * 3. 使用Command绑定按钮操作
 * 4. View只负责UI布局和样式
 */
class UserView : public PageWidget
{
    Q_OBJECT

public:
    explicit UserView(QWidget *parent = nullptr);
    ~UserView() override;

protected:
    void onLoad() override;
    void retranslateUi() override;
    void applyTheme() override;

private:
    void setupUi();
    void setupBindings();

private:
    UserViewModel *m_viewModel;

    // 表单控件
    QLineEdit  *m_nameEdit;
    QSpinBox   *m_ageSpinBox;
    QLineEdit  *m_emailEdit;
    QCheckBox  *m_activeCheckBox;

    QPushButton *m_saveButton;
    QPushButton *m_clearButton;
    QPushButton *m_loadButton;

    QLabel *m_statusLabel;
    QLabel *m_userCountLabel;

    // 需要动态翻译的控件
    QLabel     *m_titleLabel;
    QLabel     *m_descLabel;
    QGroupBox  *m_formGroup;
    QGroupBox  *m_statusGroup;
    QLabel     *m_techLabel;
    QFormLayout *m_formLayout;
};

#endif // USERVIEW_H
