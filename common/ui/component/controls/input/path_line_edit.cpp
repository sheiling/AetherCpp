/**
 * @file path_line_edit.cpp
 * @brief PathLineEdit 路径选择行与目录对话框
 */

#include "controls/input/path_line_edit.h"

#include "controls/button/push_button_style.h"
#include "controls/button/themed_push_button.h"
#include "controls/input/line_edit_style.h"
#include "controls/input/themed_line_edit.h"

#include "uibase/languagemanager.h"

#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardPaths>

namespace ui {

namespace {

bool isUncPath(const QString &path)
{
    const QString p = path.trimmed();
    return p.startsWith(QLatin1String("\\\\")) || p.startsWith(QLatin1String("//"));
}

QString safeBrowseStartDirectory(const QString &currentPath)
{
    if (!currentPath.isEmpty() && !isUncPath(currentPath)) {
        const QFileInfo fi(currentPath);
        if (fi.isDir())
            return QDir(fi.absoluteFilePath()).absolutePath();
        const QString parent = fi.absolutePath();
        if (!parent.isEmpty() && !isUncPath(parent))
            return parent;
    }
    const QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!docs.isEmpty())
        return docs;
    return QDir::homePath();
}

/** 路径专用 ThemedLineEdit（path QSS，非通用 LineEdit QSS） */
class PathEditField final : public ThemedLineEdit
{
public:
    explicit PathEditField(QWidget *parent = nullptr)
        : ThemedLineEdit(parent)
    {
        setObjectName(QStringLiteral("pathLineEdit"));
        setClearButtonEnabled(false);
    }

protected:
    void refreshStyle() override { setStyleSheet(buildPathLineEditStylesheet()); }
};

ThemedLineEdit *makePathEditField(QWidget *parent)
{
    return new PathEditField(parent);
}

} // namespace

void pickExistingDirectory(QLineEdit *edit, QWidget *dialogParent)
{
    if (!edit)
        return;

    QFileDialog dlg(dialogParent);
    dlg.setWindowTitle(TR("aoi.sys.path.browse"));
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ShowDirsOnly, true);
    dlg.setOption(QFileDialog::DontResolveSymlinks, true);
    dlg.setDirectory(safeBrowseStartDirectory(edit->text()));

    if (dlg.exec() == QDialog::Accepted) {
        const QStringList selected = dlg.selectedFiles();
        if (!selected.isEmpty())
            edit->setText(QDir::toNativeSeparators(selected.constFirst()));
    }
}

PathLineEdit::PathLineEdit(const QString &labelKey, QWidget *parent, const QString &path)
    : QWidget(parent)
    , m_labelKey(labelKey)
{
    setObjectName(QStringLiteral("pathLineEditRow"));
    buildUi(path);
    retranslateRow();

    if (LM) {
        connect(LM, &LanguageManager::languageChanged, this, [this]() { retranslateRow(); });
    }
}

void PathLineEdit::setLabelKey(const QString &key)
{
    if (key.isEmpty())
        return;
    m_labelKey = key;
    retranslateRow();
}

void PathLineEdit::setLabelWidth(int width)
{
    m_labelWidth = qMax(0, width);
    applyLabelWidth();
}

void PathLineEdit::applyLabelWidth()
{
    if (!m_label)
        return;
    if (m_labelWidth > 0) {
        m_label->setFixedWidth(m_labelWidth);
        m_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    } else {
        m_label->setMinimumWidth(0);
        m_label->setMaximumWidth(QWIDGETSIZE_MAX);
        m_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    }
}

QString PathLineEdit::path() const
{
    return m_edit ? m_edit->text() : QString();
}

void PathLineEdit::setPath(const QString &path)
{
    if (m_edit)
        m_edit->setText(path);
}

void PathLineEdit::browseForDirectory(QWidget *dialogParent)
{
    if (m_edit)
        pickExistingDirectory(m_edit, dialogParent ? dialogParent : window());
}

void PathLineEdit::buildUi(const QString &path)
{
    auto *rl = new QHBoxLayout(this);
    rl->setContentsMargins(0, 6, 0, 6);
    rl->setSpacing(10);

    m_label = new QLabel(this);
    m_label->setProperty("trKey", m_labelKey);
    m_label->setProperty("suffixColon", true);
    applyLabelWidth();

    m_edit = makePathEditField(this);
    m_edit->setText(path);
    m_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_browseBtn = makePushButton(QString(), PushButtonVariant::Secondary, this);
    m_browseBtn->setProperty("trKey", QStringLiteral("aoi.sys.path.browse"));
    m_browseBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    connect(m_browseBtn, &ThemedPushButton::clicked, this, [this]() { browseForDirectory(this); });

    rl->addWidget(m_label);
    rl->addWidget(m_edit, 1);
    rl->addWidget(m_browseBtn, 0, Qt::AlignVCenter);
}

void PathLineEdit::retranslateRow()
{
    if (m_label && !m_labelKey.isEmpty())
        m_label->setText(TR(m_labelKey.toUtf8().constData()) + QStringLiteral(":"));
    if (m_browseBtn)
        m_browseBtn->setText(TR("aoi.sys.path.browse"));
}

void PathLineEdit::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange)
        retranslateRow();
}

PathLineEdit *makePathLineEdit(const QString &labelKey, QWidget *parent, const QString &path)
{
    return new PathLineEdit(labelKey, parent, path);
}

} // namespace ui
