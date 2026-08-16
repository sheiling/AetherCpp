#pragma once

/**
 * @file theme_font.h
 * @brief 从 ACSS（material.json variables）读取字体，供 QSS 与自绘文本使用
 *
 * 提供 font_family / font_size 的读取、本机字体族解析，以及将主题字体
 * 批量应用到控件树的工具函数。字号策略与 material.css.template 一致。
 */

#include "uibase/thememanager.h"

#include <QFont>
#include <QFontDatabase>
#include <QString>
#include <QWidget>

namespace ui {

/**
 * 读取 ACSS 主题/样式变量字符串。
 * @param name 变量名（如 font_family、font_size）
 * @param fallback TM 不可用或变量为空时的默认值
 */
inline QString themeVariableOrDefault(const QString &name, const QString &fallback)
{
    if (!TM)
        return fallback;
    const QString v = TM->themeVariable(name);
    return v.isEmpty() ? fallback : v;
}

/** @return QSS font-family 串，与 material.css.template 中 {{font_family}} 一致 */
inline QString themeFontFamilyCss()
{
    return themeVariableOrDefault(QStringLiteral("font_family"),
                                  QStringLiteral("SourceHanSansCN, Roboto, sans-serif"));
}

/**
 * 解析 ACSS font_size 变量为像素字号。
 * 支持 "12px" / "12pt" 格式；解析失败时返回 12。
 */
inline int themeFontPixelSize()
{
    const QString sizeStr =
        themeVariableOrDefault(QStringLiteral("font_size"), QStringLiteral("12px"));
    QString num = sizeStr.trimmed();
    num.remove(QStringLiteral("px"), Qt::CaseInsensitive);
    num.remove(QStringLiteral("pt"), Qt::CaseInsensitive);
    bool ok     = false;
    const int px = num.toInt(&ok);
    return (ok && px > 0) ? px : 12;
}

/**
 * 获取 UI 字号（磅值）。
 * @param overridePt 若 > 0 则直接返回该磅值，否则等同 themeFontPixelSize()
 */
inline int themeFontPointSize(int overridePt = 0)
{
    if (overridePt > 0)
        return overridePt;
    return themeFontPixelSize();
}

/**
 * 解析 font_family 候选列表，返回本机 QFontDatabase 中已安装的第一个字体族。
 * 跳过 sans-serif / serif / monospace 等 CSS 泛型族名。
 */
inline QString themeResolvedFontFamily()
{
    const QStringList candidates =
        themeFontFamilyCss().split(QLatin1Char(','), Qt::SkipEmptyParts);
    QFontDatabase db;
    for (QString raw : candidates) {
        QString name = raw.trimmed();
        if (name.startsWith(QLatin1Char('"')) && name.endsWith(QLatin1Char('"')))
            name = name.mid(1, name.size() - 2);
        if (name.compare(QLatin1String("sans-serif"), Qt::CaseInsensitive) == 0
            || name.compare(QLatin1String("serif"), Qt::CaseInsensitive) == 0
            || name.compare(QLatin1String("monospace"), Qt::CaseInsensitive) == 0)
            continue;
        if (db.hasFamily(name))
            return name;
    }
    if (!candidates.isEmpty()) {
        QString name = candidates.first().trimmed();
        if (name.startsWith(QLatin1Char('"')) && name.endsWith(QLatin1Char('"')))
            name = name.mid(1, name.size() - 2);
        return name;
    }
    return {};
}

/**
 * 构造与全局 ACSS 一致的 UI 字体。
 * @param overridePt 若 > 0 使用磅值字号；否则使用 ACSS font_size（像素）
 */
inline QFont themeUiFont(int overridePt = 0)
{
    QFont f;
    const QString family = themeResolvedFontFamily();
    if (!family.isEmpty())
        f.setFamily(family);
    if (overridePt > 0)
        f.setPointSize(overridePt);
    else
        f.setPixelSize(themeFontPixelSize());
    return f;
}

/**
 * 将主题字体递归应用到 root 及其所有 QWidget 子节点。
 * @param root 根控件；为 nullptr 时无操作
 * @param overridePt 传递给 themeUiFont 的字号覆盖
 */
inline void applyThemeUiFontTree(QWidget *root, int overridePt = 0)
{
    if (!root)
        return;
    const QFont font = themeUiFont(overridePt);
    root->setFont(font);
    for (auto *child : root->findChildren<QWidget *>())
        child->setFont(font);
}

} // namespace ui
