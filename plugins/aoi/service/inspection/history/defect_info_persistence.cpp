/**
 * @file defect_info_persistence.cpp
 * @brief DefectInfoList JSON 序列化落盘
 */

#include "defect_info_persistence.h"

#include "defect_info_codec.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

namespace aoi::inspection {

bool saveDefectInfoList(const DefectInfoList &list, QString *errorOut)
{
    const QString path = list.defectInfoFullFileName.trimmed();
    if (path.isEmpty()) {
        if (errorOut)
            *errorOut = QStringLiteral("defectInfoFullFileName is empty");
        return false;
    }

    QDir().mkpath(QFileInfo(path).absolutePath());

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorOut)
            *errorOut = file.errorString();
        return false;
    }

    if (file.write(defectInfoListToJson(list).toUtf8()) < 0) {
        if (errorOut)
            *errorOut = file.errorString();
        return false;
    }
    return true;
}

} // namespace aoi::inspection
