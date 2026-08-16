#pragma once

/**
 * @file die_handle_pool.h
 * @brief 固晶 CISDK 句柄池（VisionAlgo 插件内部，DieBondingDetectEngine 持有）
 *
 * 生命周期：
 * - 构造：按 initialPoolSize 预创建句柄并 loadModels / setParam
 * - GetHandle：线程绑定复用；初始池满时动态扩容（push_back 新句柄）
 * - SetParam：刷新全部有效句柄的像素/阈值参数（不重载模型文件）
 * - 析构：销毁 vector 内全部 CISDK 句柄
 *
 * 线程模型：每个 std::thread 绑定一个 AlgoHandle（m_threadID）；CISDK 要求同线程 create/use。
 */

#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include <common/utility/errors/include/error.h>

#include <die_bonding_defect_detector.h>

#include <mutex>
#include <string>
#include <vector>

namespace vision_algo {

class HandlePool
{
public:
    /**
     * @param poolSize        初始句柄数量（m_initialPoolSize，运行期不变）
     * @param isDeepLearning  true → DLDieBonding* API；false → 传统 DieBonding*
     * @param param           芯片/焊盘尺寸、模型路径、DL 推理引擎等
     * @param dlModelDir      DL 模型目录；chipGroupModel 为空时自动扫描
     */
    HandlePool(size_t poolSize, bool isDeepLearning, const DieBondingChipDetectParam &param,
               const std::string &dlModelDir = "./dlmodel/");
    ~HandlePool();

    HandlePool(const HandlePool &)            = delete;
    HandlePool &operator=(const HandlePool &) = delete;

    /** 构造时设定的初始容量（配置值，不随扩容改变） */
    std::size_t initialPoolSize() const { return m_initialPoolSize; }

    /** 当前句柄槽位总数（含运行期动态扩容） */
    std::size_t handleCount() const { return m_handles.size(); }

    /**
     * @brief 获取当前线程绑定的 CISDK 句柄
     * @return 有效 AlgoHandle；失败返回 nullptr（扩容失败或 createHandle 错误）
     */
    AlgoHandle GetHandle();

    /**
     * @brief 将新参数同步到池内全部有效句柄
     * @return nullptr 成功；否则首个失败的 ErrorPtr
     */
    common::errors::ErrorPtr SetParam(const DieBondingChipDetectParam &param);

private:
    common::errors::ErrorPtr createHandle(AlgoHandle &handle);
    void                     destroyHandle(AlgoHandle handle);
    common::errors::ErrorPtr setParamModel(AlgoHandle handle);
    common::errors::ErrorPtr setDieParamDL(AlgoHandle handle);
    common::errors::ErrorPtr setDieParam(AlgoHandle handle);
    common::errors::ErrorPtr loadModelsDL(AlgoHandle handle);
    common::errors::ErrorPtr loadModels(AlgoHandle handle);

    /** 单槽：CISDK 句柄 + 绑定线程 id（0=未绑定）+ 是否可用 */
    struct AlgoHandleStruct
    {
        AlgoHandle m_handle   = nullptr;
        int        m_threadID = 0;
        bool       m_valid    = false;
    };

    std::vector<AlgoHandleStruct> m_handles;
    std::size_t                   m_initialPoolSize = 0;
    mutable std::mutex            m_mutex;
    bool                          m_isDeepLearning = false;
    DieBondingChipDetectParam     m_param;
    std::string                   m_dlModelDir;
    /** loadModelsDL 各模型类型加载互斥（CHIP_GROUP / POLARITY / DETECT / CLASSIFY） */
    std::mutex                    m_szmutex[4];
};

} // namespace vision_algo
