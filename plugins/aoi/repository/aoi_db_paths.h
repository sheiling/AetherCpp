#pragma once

/**
 * @file aoi_db_paths.h
 * @brief AOI 业务 SQLite 默认路径（与 permission.db 同目录）
 */

#include <QCoreApplication>
#include <QDir>
#include <QString>

namespace aoi::repository {

/** @brief 默认 record.db：{exe}/data/record.db（检测历史 + 参数审计同库） */
inline QString defaultRecordDbPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("data/record.db"));
}

} // namespace aoi::repository
