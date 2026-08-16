#pragma once

/**
 * @file recipe_scan_detect_view_model.h
 * @brief 配方配置页扫码子 ViewModel：预览帧 ZXing 识别
 */

#include <vision_algo/units/scan/barcode_scan_data.h>

#include <QImage>
#include <QObject>

class FunctionalCommand;

namespace aoi {

class RecipeConfigViewModel;

/**
 * @brief 配方页条码/二维码识别
 *
 * 预览帧经 `image_display::qImageToRgbImageInfo` 送 VisionAlgo BarcodeScan；
 * 运行状态与结果由本 VM 信号对外暴露，全局 status 仍经宿主 `setStatus`。
 */
class RecipeScanDetectViewModel : public QObject
{
    Q_OBJECT

public:
    explicit RecipeScanDetectViewModel(RecipeConfigViewModel *host, QObject *parent = nullptr);

    bool                           scanDetecting() const { return m_scanDetecting; }
    vision_algo::BarcodeScanResult scanResult() const { return m_scanResult; }
    FunctionalCommand             *scanDetectCommand() const { return m_scanDetectCommand; }

public slots:
    void scanDetectFromPreview(int winId);
    void scanDetectFromImage(const QImage &image);

signals:
    void scanDetectingChanged();
    void scanResultChanged();

private slots:
    void tickRecognizingAnimation();
    void finishScanDetect(const vision_algo::BarcodeScanResult &result);

private:
    void setScanDetecting(bool detecting);
    void startRecognizingAnimation();
    void stopRecognizingAnimation();

    RecipeConfigViewModel *m_host              = nullptr;
    bool                   m_scanDetecting     = false;
    FunctionalCommand     *m_scanDetectCommand = nullptr;
    vision_algo::BarcodeScanResult m_scanResult;

    class QTimer *m_recognizingTimer    = nullptr;
    int           m_recognizingDotCount = 0;
};

} // namespace aoi
