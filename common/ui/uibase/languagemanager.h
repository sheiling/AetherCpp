#pragma once

#include "uibase_global.h"
#include <QObject>
#include <QString>
#include <QHash>
#include <memory>
#include <vector>
#include <QTranslator>

/**
 * @brief 国际化语言管理器（单例）
 */
class UIBASE_EXPORT LanguageManager : public QObject
{
    Q_OBJECT

public:
    enum class Language { Chinese, English };
    Q_ENUM(Language)

    static LanguageManager *instance();
    ~LanguageManager();

    Language currentLanguage() const { return m_language; }
    bool isChinese() const { return m_language == Language::Chinese; }

    void setLanguage(Language lang);
    void toggleLanguage();

    QString t(const QString &key) const;
    QString t(const QString &key, const QString &arg1) const;

    /** @brief 注册插件翻译（qm 文件名为 {qmBaseName}_zh_CN.qm / {qmBaseName}_en_US.qm） */
    void registerPluginTranslator(const QString &pluginId, const QString &qmBaseName);

signals:
    void languageChanged(Language newLang);

private:
    explicit LanguageManager(QObject *parent = nullptr);
    bool loadLanguageTranslator(Language lang);
    void unloadAllTranslators();
    bool loadAppTranslator(Language lang);
    void loadPluginTranslators(Language lang);
    QStringList translationSearchPaths() const;

    Language m_language;
    std::unique_ptr<QTranslator> m_translator;
    QStringList                  m_pluginQmBases;
    QHash<QString, QString>      m_pluginQmBaseById;
    std::vector<std::unique_ptr<QTranslator>> m_pluginTranslators;
};

#define LM  (LanguageManager::instance())
#define TR(key) (LanguageManager::instance()->t(key))
