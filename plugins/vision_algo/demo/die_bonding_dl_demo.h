#pragma once

#include <QCommandLineParser>
#include <QString>

namespace aoi {
namespace demo {

int listDlModels(const QString &dlModelDir);
int runDieBondingDl(const QCommandLineParser &parser);

} // namespace demo
} // namespace aoi
