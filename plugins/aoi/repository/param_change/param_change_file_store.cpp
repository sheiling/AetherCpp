#include "param_change_file_store.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

namespace aoi {
namespace {

/**
 * @brief 默认文本日志目录：{exe}/parameter_change
 */
QString defaultLogDir()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/parameter_change");
}

QString dailyLogPath(const QString &logDir)
{
    const QString dir = logDir.trimmed().isEmpty() ? defaultLogDir() : logDir;
    if (!QDir().mkpath(dir))
        return {};
    return dir + QLatin1Char('/')
           + QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")) + QStringLiteral(".txt");
}

void setUtf8(QTextStream &out)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif
}

} // namespace

ParamChangeFileStore::ParamChangeFileStore(const QString &logDir)
    : m_logDir(logDir.trimmed())
{
}

void ParamChangeFileStore::save(const ParamAuditContext &ctx,
                                const QVector<parammeta::ParamChangeRecord> &changes)
{
    // 文本落盘策略：按天分文件，便于现场排查与归档。
    if (changes.isEmpty())
        return;
    const QString path = dailyLogPath(m_logDir);
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream out(&file);
    setUtf8(out);
    out << QStringLiteral(
               "================================================================================\n");
    out << QStringLiteral("时间: ")
        << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
        << QLatin1Char('\n');
    out << QStringLiteral("模块: ") << ctx.moduleCode << QLatin1Char('\n');
    out << QStringLiteral("类型: ") << ctx.documentType << QLatin1Char('\n');
    if (!ctx.documentId.isEmpty())
        out << QStringLiteral("文档ID: ") << ctx.documentId << QLatin1Char('\n');
    out << QStringLiteral("页面: ") << ctx.sourcePage << QLatin1Char('\n');
    out << QStringLiteral("--- 参数变更明细 ---\n");
    for (const parammeta::ParamChangeRecord &change : changes) {
        out << change.fullLabel << QStringLiteral(" [") << change.labelPattern << QStringLiteral("] : ")
            << change.beforeValue << QStringLiteral(" -> ") << change.afterValue << QLatin1Char('\n');
    }
    out << QLatin1Char('\n');
}

} // namespace aoi
