#include "languagemanager.h"

#include "translation_contexts.generated.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QStringList>
#include <QTranslator>

LanguageManager *LanguageManager::instance()
{
    static LanguageManager inst;
    return &inst;
}

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent)
    , m_language(Language::Chinese)
{
    m_translator = std::make_unique<QTranslator>(this);
    loadLanguageTranslator(m_language);
}

LanguageManager::~LanguageManager() = default;

void LanguageManager::setLanguage(Language lang)
{
    if (m_language != lang) {
        loadLanguageTranslator(lang);
        m_language = lang;
        emit languageChanged(lang);
    }
}

void LanguageManager::toggleLanguage()
{
    setLanguage(m_language == Language::Chinese ? Language::English : Language::Chinese);
}

void LanguageManager::registerPluginTranslator(const QString &pluginId,
                                               const QString &qmBaseName)
{
    if (pluginId.isEmpty() || qmBaseName.isEmpty()) {
        return;
    }

    if (m_pluginQmBaseById.contains(pluginId)) {
        return;
    }

    m_pluginQmBaseById.insert(pluginId, qmBaseName);
    if (!m_pluginQmBases.contains(qmBaseName)) {
        m_pluginQmBases.append(qmBaseName);
    }

    loadPluginTranslators(m_language);
}

namespace {

QString translateKeyInContexts(const QString &key)
{
    const QByteArray source = key.toUtf8();
    // context 列表由 CMake 从 config/app_*.ts 与 plugins/*/i18n/*.ts 自动生成
    for (const QString &ctx : translation_contexts::all()) {
        const QString translated =
            QCoreApplication::translate(ctx.toUtf8().constData(), source.constData());
        if (!translated.isEmpty() && translated != key)
            return translated;
    }
    return key;
}

} // namespace

QString LanguageManager::t(const QString &key) const
{
    return translateKeyInContexts(key);
}

QString LanguageManager::t(const QString &key, const QString &arg1) const
{
    return t(key).arg(arg1);
}

QStringList LanguageManager::translationSearchPaths() const
{
    return {
        QCoreApplication::applicationDirPath() + "/config/i18n",
        QCoreApplication::applicationDirPath() + "/config",
        QCoreApplication::applicationDirPath() + "/i18n",
        QCoreApplication::applicationDirPath() + "/translations",
        ":/i18n",
        ":/translations"
    };
}

void LanguageManager::unloadAllTranslators()
{
    if (m_translator) {
        QCoreApplication::removeTranslator(m_translator.get());
    }
    for (auto &translator : m_pluginTranslators) {
        if (translator) {
            QCoreApplication::removeTranslator(translator.get());
        }
    }
    m_pluginTranslators.clear();
}

bool LanguageManager::loadAppTranslator(Language lang)
{
    const QString qmFileName = (lang == Language::Chinese) ? "app_zh_CN.qm" : "app_en_US.qm";

    for (const QString &basePath : translationSearchPaths()) {
        const QString fullPath = basePath + "/" + qmFileName;
        if (QFileInfo::exists(fullPath) && m_translator && m_translator->load(fullPath)) {
            QCoreApplication::installTranslator(m_translator.get());
            return true;
        }
    }

    return false;
}

void LanguageManager::loadPluginTranslators(Language lang)
{
    for (auto &translator : m_pluginTranslators) {
        if (translator) {
            QCoreApplication::removeTranslator(translator.get());
        }
    }
    m_pluginTranslators.clear();

    const QString langSuffix = (lang == Language::Chinese) ? "zh_CN" : "en_US";

    for (const QString &qmBaseName : m_pluginQmBases) {
        const QString qmFileName = qmBaseName + "_" + langSuffix + ".qm";
        auto translator = std::make_unique<QTranslator>(this);

        bool loaded = false;
        for (const QString &basePath : translationSearchPaths()) {
            const QString fullPath = basePath + "/" + qmFileName;
            if (QFileInfo::exists(fullPath) && translator->load(fullPath)) {
                QCoreApplication::installTranslator(translator.get());
                loaded = true;
                break;
            }
        }

        if (loaded) {
            m_pluginTranslators.push_back(std::move(translator));
        }
    }
}

bool LanguageManager::loadLanguageTranslator(Language lang)
{
    unloadAllTranslators();
    const bool appLoaded = loadAppTranslator(lang);
    loadPluginTranslators(lang);
    return appLoaded;
}
