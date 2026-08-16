#include <stdio.h>
#include "../image_capture.h"
#include <3rdparty/opencv_4.9.0/include/opencv2/opencv.hpp>
#include "../../deviceManager/device_manager.h"

void saveAlgoMatAsJPG(const AlgoMat& mat, const std::string& filename) {
    // 创建 OpenCV Mat 对象，假设矩阵类型是 CV_8UC1 (灰度图像)
    cv::Mat image(mat.rows, mat.cols, CV_8UC1, mat.data);

    // 保存图像为 JPG 文件
    if (!image.empty()) {
        if (cv::imwrite(filename, image)) {
            std::cout << "Image saved successfully to " << filename << std::endl;
        }
        else {
            std::cerr << "Failed to save image to " << filename << std::endl;
        }
    }
}


int main() {
    std::shared_ptr<common::device::DeviceManager> deviceManager = std::make_shared<common::device::DeviceManager>();
    REGISTER_MANAGER->Register(deviceManager);
    REGISTER_MANAGER->Load(deviceManager->getName());

    // 2 设备管理器初始化
    int ret = deviceManager->Init();
    if (ret != 0) {
        return -1;
    }
    REGISTER_MANAGER->Save();
    std::shared_ptr<common::device::ImageCapture> imageCapture = deviceManager->ImageCaptureHandler("iamgeCaptureName");
    if (imageCapture) {
        for (int i = 0; i < 5; ++i) {
            imageCapture->captureImage(false);
            std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();
            // 等待到达延迟时间
            while (true) {
                if (std::chrono::high_resolution_clock::now() - startTime >= (std::chrono::milliseconds)200/*imageCapture->m_delayTime*/)
                    break;
                std::this_thread::yield();
            }
        }
        for (int i = 0; i < 5; ++i) {
            std::shared_ptr<AlgoMat> testMat = imageCapture->getImage();
            if (testMat) {
                std::string str = "output";
                str.append(std::to_string(i));
                str.append(".jpg");
                saveAlgoMatAsJPG(*testMat, str);
                std::cout << "saveAlgoMatAsJPG -- " << str << std::endl;
                AlgoMatFree(testMat.get());
            }
        }

        getchar();
        imageCapture->close();
    }
    else
    {
        std::cout << "imageCapture is null " << std::endl;
    }

    return 0;
}
