#include "vision_algo_i18n.h"

#include <QCoreApplication>

namespace vision_algo {

QString tr(const char *key)
{
    if (!key || !key[0])
        return {};
    const QString source = QString::fromUtf8(key);
    const QString translated =
        QCoreApplication::translate("VisionAlgo", source.toUtf8().constData());
    return translated.isEmpty() || translated == source ? source : translated;
}

} // namespace vision_algo
