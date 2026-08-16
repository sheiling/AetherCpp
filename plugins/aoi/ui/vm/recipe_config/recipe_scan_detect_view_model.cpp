/**
 * @file recipe_scan_detect_view_model.cpp
 */

#include "recipe_scan_detect_view_model.h"
#include "recipe_config_view_model.h"

#include "image/image_info_display.h"

#include <vision_algo/core/service_access.h>
#include <vision_algo/units/scan/barcode_scan_data.h>

#include <command.h>
#include <uibase/languagemanager.h>

#include <QTimer>

namespace aoi {

RecipeScanDetectViewModel::RecipeScanDetectViewModel(RecipeConfigViewModel *host, QObject *parent)
    : QObject(parent)
    , m_host(host)
{
    m_scanDetectCommand = new FunctionalCommand(
        [this]() { scanDetectFromPreview(2); },
        [this]() { return m_host && !m_scanDetecting; },
        this);
    m_scanDetectCommand->setName(QStringLiteral("recipe.scanDetect"));

    m_recognizingTimer = new QTimer(this);
    m_recognizingTimer->setInterval(400);
    connect(m_recognizingTimer, &QTimer::timeout, this,
            &RecipeScanDetectViewModel::tickRecognizingAnimation);
}

void RecipeScanDetectViewModel::setScanDetecting(bool detecting)
{
    if (m_scanDetecting == detecting)
        return;
    m_scanDetecting = detecting;
    emit scanDetectingChanged();
}

void RecipeScanDetectViewModel::scanDetectFromPreview(int winId)
{
    Q_UNUSED(winId);
    if (!m_host)
        return;
    scanDetectFromImage(m_host->lastPreviewImage());
}

void RecipeScanDetectViewModel::scanDetectFromImage(const QImage &image)
{
    if (!m_host || m_scanDetecting)
        return;

    if (image.isNull()) {
        m_host->notifyStatus(TR("aoi.recipe.scan.no_preview"));
        return;
    }

    m_host->setLastPreviewImage(image);
    setScanDetecting(true);
    startRecognizingAnimation();

    vision_algo::VisionImageContext vision;
    vision.processImage = image_display::qImageToRgbImageInfo(m_host->lastPreviewImage());
    vision_algo::BarcodeScanInput inPkt;
    inPkt.setVision(std::move(vision));
    common::algorithm::AlgorithmExecuteData data;
    data.inputs = static_cast<const common::algorithm::AlgorithmPacket &>(inPkt);
    if (auto *svc = vision_algo::service()) {
        if (const auto err = svc->execute(vision_algo::kUnitBarcodeScan, data); err) {
            m_host->notifyStatus(QString::fromStdString(err->Message()));
            setScanDetecting(false);
            stopRecognizingAnimation();
            return;
        }
    }
    auto *out = data.outputs.Get<vision_algo::BarcodeScanResult>(vision_algo::BarcodeScanOutput::kResult);
    finishScanDetect(out ? *out : vision_algo::BarcodeScanResult{});
}

void RecipeScanDetectViewModel::startRecognizingAnimation()
{
    if (!m_host)
        return;
    m_recognizingDotCount = 0;
    m_host->setStatus(TR("aoi.recipe.scan.recognizing"));
    m_recognizingTimer->start();
}

void RecipeScanDetectViewModel::stopRecognizingAnimation()
{
    m_recognizingTimer->stop();
    m_recognizingDotCount = 0;
}

void RecipeScanDetectViewModel::tickRecognizingAnimation()
{
    if (!m_host)
        return;
    m_recognizingDotCount = (m_recognizingDotCount + 1) % 4;
    const QString dots    = QString(m_recognizingDotCount, QChar(0x3002));
    m_host->setStatus(TR("aoi.recipe.scan.recognizing") + dots);
}

void RecipeScanDetectViewModel::finishScanDetect(const vision_algo::BarcodeScanResult &result)
{
    if (!m_host)
        return;

    stopRecognizingAnimation();
    setScanDetecting(false);

    m_scanResult = result;
    emit scanResultChanged();

    if (m_scanResult.ok()) {
        const QString typeLabel = m_scanResult.codeType == vision_algo::BarcodeCodeType::QrCode
                                    ? QStringLiteral("QR")
                                    : (m_scanResult.symbology.isEmpty()
                                           ? QStringLiteral("Barcode")
                                           : m_scanResult.symbology);
        m_host->setStatus(TR("aoi.recipe.scan.success").arg(typeLabel, m_scanResult.content));
    } else {
        m_host->setStatus(TR("aoi.recipe.scan.failed"));
    }
}

} // namespace aoi
