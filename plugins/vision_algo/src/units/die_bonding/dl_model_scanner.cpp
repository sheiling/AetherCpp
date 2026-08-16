#include "dl_model_scanner.h"

#include "vision_algo_i18n.h"

/**
 * @file dl_model_scanner.cpp
 * @brief dlmodel 目录扫描（仅 DL 固晶使用）
 */

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace vision_algo {
namespace {

QString readTextFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QString::fromUtf8(file.readAll()).trimmed();
}

QString normalizeDirPath(QString path)
{
    path = QDir::fromNativeSeparators(path.trimmed());
    if (path.isEmpty())
        return path;
    if (!path.endsWith(QLatin1Char('/')))
        path += QLatin1Char('/');
    return path;
}

QString defaultDlModelDirectory()
{
    const QString besideExe =
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("dlmodel"));
    if (QDir(besideExe).exists())
        return QDir(besideExe).absolutePath();
    return besideExe;
}

bool startsWith(const QString &name, const QString &prefix)
{
    return name.startsWith(prefix, Qt::CaseInsensitive);
}

QString extension(const QString &name)
{
    const int dot = name.lastIndexOf(QLatin1Char('.'));
    if (dot < 0 || dot == name.size() - 1)
        return {};
    return name.mid(dot + 1).toLower();
}

void assignModelPath(std::string &target, const QString &fullPath, bool preferOnnx, const QString &ext,
                     QStringList *outFound, const QString &fileName)
{
    if (target.empty()) {
        target = fullPath.toStdString();
        if (outFound)
            outFound->append(fileName);
        return;
    }
    if (!preferOnnx)
        return;

    const QString currentExt = QString::fromStdString(target).section(QLatin1Char('.'), -1).toLower();
    if (currentExt == QLatin1String("trt") && ext == QLatin1String("onnx")) {
        target = fullPath.toStdString();
        if (outFound)
            outFound->append(fileName);
    }
}

} // namespace

bool scanDlModelDirectory(const QString &directory, DieBondingChipDetectParam &param,
                          QStringList *outFound, QString *error, bool preferOnnx)
{
    const QString resolvedDir =
        directory.trimmed().isEmpty() ? defaultDlModelDirectory() : directory;
    const QString dirPath = normalizeDirPath(resolvedDir);
    if (dirPath.isEmpty() || !QDir(dirPath).exists()) {
        if (error)
            *error = tr("vision_algo.error.dl_model_directory_not_found").arg(resolvedDir);
        return false;
    }

    param.chipGroupModel.clear();
    param.chipGroupKey.clear();
    param.chipDetectModel.clear();
    param.chipDetectKey.clear();
    param.polarDetectModel.clear();
    param.polarDetectKey.clear();
    param.detectClassifyModel.clear();
    param.detectClassifyKey.clear();

    const QDir dir(dirPath);
    const QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &info : files) {
        const QString fileName = info.fileName();
        const QString ext      = extension(fileName);
        const QString fullPath = info.absoluteFilePath();

        if (ext == QLatin1String("onnx") || ext == QLatin1String("trt")) {
            if (startsWith(fileName, QStringLiteral("chip_group_encry"))) {
                assignModelPath(param.chipGroupModel, fullPath, preferOnnx, ext, outFound, fileName);
            } else if (startsWith(fileName, QStringLiteral("chip_detect_encry"))) {
                assignModelPath(param.chipDetectModel, fullPath, preferOnnx, ext, outFound, fileName);
            } else if (startsWith(fileName, QStringLiteral("polar_detect_encry"))) {
                assignModelPath(param.polarDetectModel, fullPath, preferOnnx, ext, outFound, fileName);
            } else if (startsWith(fileName, QStringLiteral("defect_cls_encry"))) {
                assignModelPath(param.detectClassifyModel, fullPath, preferOnnx, ext, outFound, fileName);
            }
        } else if (ext == QLatin1String("txt")) {
            const QString key = readTextFile(fullPath);
            if (key.isEmpty())
                continue;
            if (startsWith(fileName, QStringLiteral("chip_group_encry"))) {
                param.chipGroupKey = key.toStdString();
                if (outFound)
                    outFound->append(fileName);
            } else if (startsWith(fileName, QStringLiteral("chip_detect_encry"))) {
                param.chipDetectKey = key.toStdString();
                if (outFound)
                    outFound->append(fileName);
            } else if (startsWith(fileName, QStringLiteral("polar_detect_encry"))) {
                param.polarDetectKey = key.toStdString();
                if (outFound)
                    outFound->append(fileName);
            } else if (startsWith(fileName, QStringLiteral("defect_cls_encry"))) {
                param.detectClassifyKey = key.toStdString();
                if (outFound)
                    outFound->append(fileName);
            }
        }
    }

    if (param.chipGroupModel.empty() || param.chipGroupKey.empty() || param.chipDetectModel.empty()
        || param.chipDetectKey.empty()) {
        if (error)
            *error = tr("vision_algo.error.dl_model_required_files_missing");
        return false;
    }

    return true;
}

} // namespace vision_algo
