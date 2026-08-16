#ifndef ALGO_MAT_SHARED_H
#define ALGO_MAT_SHARED_H

/**
 * @file algo_mat_shared.h
 * @brief AlgoMat 的 shared_ptr 封装：析构时调用 AlgoMatFree，避免像素缓冲泄漏
 *
 * CISDK 的 AlgoMatCreate / AlgoMatCreateFromMem 分配的 data 必须用 AlgoMatFree 释放，
 * 不能使用 shared_ptr 默认 delete。
 */

#include <common/3rdparty/cisdk/measurement/include/algo_mat.h>

#include <memory>

namespace common {
namespace device {

inline void clearAlgoMatHeader(AlgoMat &mat) noexcept
{
    mat.rows = 0;
    mat.cols = 0;
    mat.type = ALGO_8UC1;
    mat.len  = 0;
    mat.data = nullptr;
}

struct AlgoMatDeleter {
    void operator()(AlgoMat *mat) const noexcept
    {
        if (!mat) {
            return;
        }
        if (!AlgoMatIsEmpty(mat)) {
            AlgoMatFree(mat);
        }
        delete mat;
    }
};

using SharedAlgoMat = std::shared_ptr<AlgoMat>;

/** 将栈上/局部 AlgoMat 所有权移入 shared_ptr（调用后 donor 头置空，不可再 Free） */
inline SharedAlgoMat adoptAlgoMat(AlgoMat mat)
{
    if (AlgoMatIsEmpty(&mat)) {
        return nullptr;
    }
    auto *heapMat = new AlgoMat(mat);
    clearAlgoMatHeader(mat);
    return SharedAlgoMat(heapMat, AlgoMatDeleter{});
}

/**
 * 从 SharedAlgoMat 剥离像素所有权到栈上 AlgoMat。
 * AlgoMat 为 C POD，std::move(*ptr) 只会拷贝指针字段；须先 clearAlgoMatHeader 再 reset，
 * 否则析构会对同一块 data 二次 AlgoMatFree。
 */
inline AlgoMat detachAlgoMat(SharedAlgoMat &holder)
{
    if (!holder || AlgoMatIsEmpty(holder.get())) {
        holder.reset();
        return {};
    }
    AlgoMat out = *holder;
    clearAlgoMatHeader(*holder);
    holder.reset();
    return out;
}

} // namespace device
} // namespace common

#endif // ALGO_MAT_SHARED_H
