#ifndef __COMMON_UTILITY_FILE_CLEANUP_H__
#define __COMMON_UTILITY_FILE_CLEANUP_H__

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <filesystem>

namespace common
{
namespace utility
{

//add by liuyuying 2024.04.17  Attention please this filesystem namespace must be depend on c++17
namespace fs = std::filesystem;
// 文件信息结构体，用于存储文件信息
struct FileInfo {
    std::string filename;
    std::time_t timestamp;
    uintmax_t  size;
};

class FileCleanup {
public:
    static FileCleanup& GetInstance()
    {
        static FileCleanup fileCleanup;
        return fileCleanup;
    }  

    // 启动清理线程
    void StartCleanupThread() {
        if (stopCleanup_)
        {
            cleanupThread_ = std::thread(&FileCleanup::CleanupThreadFunction, this);
            stopCleanup_ = false;
        }
    }

    // 停止清理线程
    void StopCleanupThread() {
        stopCleanup_ = true;
        if (cleanupThread_.joinable()) {
            cleanupThread_.join();
        }
    }

    void InitCleanupFileParm(const size_t maxMemory, const double maxMemoryPercent, const std::string& folder)
    {
        if (maxMemoryPercent > 1.0)
            maxMemoryPercent_ = 1.0;
        else if (maxMemoryPercent < 0)
            maxMemoryPercent_ = 0;
        else
            maxMemoryPercent_ = maxMemoryPercent;
    }

private:
    FileCleanup() {};
    ~FileCleanup() {};
    FileCleanup(const FileCleanup&) = delete;
    FileCleanup& operator=(const FileCleanup&) = delete;

    // 清理线程函数
    void CleanupThreadFunction() {
        while (!stopCleanup_) {
            std::this_thread::sleep_for(std::chrono::seconds(10)); // 每隔一段时间执行一次清理操作，这里设置为10秒

            std::lock_guard<std::mutex> lock(mutex_);
            files_.clear();
            GetAllFiles(folder_, files_);
            currentMemory_ = GetCurrentMemoryUsage();
            std::cout << "Current memory usage: " << currentMemory_ << "Current thread id: " << cleanupThread_.get_id() << std::endl;

            if (currentMemory_ > maxMemory_) {
                std::cout << "Memory exceeds limit. Starting cleanup..." << std::endl;
                CleanupFiles();
            }
        }
    }

    // 获取当前内存使用量
    size_t GetCurrentMemoryUsage() {
        size_t totalSize = 0;
        for (const auto& file : files_) {
            totalSize += file.size;
        }
        return totalSize;
    }

    // 清理文件
    void CleanupFiles() {
        //排序最早写入的文件排最前面
        std::sort(files_.begin(), files_.end(), [](const FileInfo& a, const FileInfo& b) {
            return a.timestamp < b.timestamp;
            });

        size_t removedMemory = 0;
        auto it = files_.begin();
        while (it != files_.end()) {
            std::remove(it->filename.c_str());
            removedMemory += it->size;
            std::cout << "Removed file is:" << it->filename.c_str() << std::endl;
            it = files_.erase(it);
            //保留最大容量的百分比大小的文件
            if (currentMemory_ - removedMemory < maxMemory_ * maxMemoryPercent_)
                break;
        }
        std::cout << "Cleanup completed. Removed " << removedMemory << " bytes." << std::endl;
    }

    // 递归地获取指定路径下的所有文件信息
    void GetAllFiles(const fs::path& root, std::vector<FileInfo>& files) {
        try
        {           
            for (const auto& entry : fs::directory_iterator(root)) {
                if (fs::is_regular_file(entry)) {
                    FileInfo fileInfo;
                    fileInfo.filename = entry.path().string();
                    fileInfo.timestamp = fs::last_write_time(entry).time_since_epoch().count();
                    fileInfo.size = fs::file_size(entry);
                    files.push_back(fileInfo);
                }
                else if (fs::is_directory(entry)) {
                    GetAllFiles(entry.path(), files); // 递归处理子文件夹
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }

    }

private:
    size_t maxMemory_; // 最大内存容量
    std::string folder_; // 文件夹路径
    std::vector<FileInfo> files_; // 文件信息列表
    std::thread cleanupThread_; // 清理线程
    std::mutex mutex_; // 互斥锁
    bool stopCleanup_ = true; // 是否停止清理线程
    double maxMemoryPercent_; // 清理最大内存容量百分比大小的文件
    size_t currentMemory_;  //当前文件夹内的容量
};

}
}

#if 0
int main() {
    //DEMO测试路径
    fs::path currentPath = fs::current_path();
    fs::path folderPath = currentPath / "path";
    // 创建CleanupFile对象
    auto& fileCleanup = FileCleanup::GetInstance();
    fileCleanup.InitCleanupFileParm(1000000, 0.5, folderPath.string());
    // 启动清理线程
    fileCleanup.StartCleanupThread();
    // 停止清理线程
    for (int i = 0; i < 10000; ++i)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    fileCleanup.StopCleanupThread();

    return 0;
}
#endif

#endif
