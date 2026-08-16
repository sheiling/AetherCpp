#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <common/base/algorithm/algorithm_global.h>

namespace common
{
namespace algorithm
{

// ---------------------------------------------------------------------------
// ImageBuffer — 零拷贝图像缓冲区
//
// 用 shared_ptr 持有裸内存所有权。算法间传递时只复制指针，图像数据不拷贝。
// 与 OpenCV Mat 互转只需封装头信息（data 指针复用）。
// ---------------------------------------------------------------------------
struct ImageBuffer
{
    int width     = 0;
    int height    = 0;
    int channels  = 0;
    int elem_size = 1;   // 每通道每像素字节数：uint8=1, uint16=2, float32=4
    int step      = 0;   // 每行字节数（含对齐 padding）

    // 共享内存所有权：多个 shared_ptr<ImageBuffer> 指向同一像素数据
    std::shared_ptr<uint8_t[]> data;

    bool   Empty()    const noexcept { return !data || width == 0 || height == 0; }
    size_t ByteSize() const noexcept
    {
        return static_cast<size_t>(step) * static_cast<size_t>(height);
    }

    // 分配新缓冲区（step=0 时自动按行紧密排布）
    static std::shared_ptr<ImageBuffer> Create(
        int w, int h, int c, int elem_size = 1, int step = 0)
    {
        auto buf      = std::make_shared<ImageBuffer>();
        buf->width    = w;
        buf->height   = h;
        buf->channels = c;
        buf->elem_size = elem_size;
        buf->step     = (step > 0) ? step : w * c * elem_size;
        buf->data     = std::shared_ptr<uint8_t[]>(new uint8_t[buf->ByteSize()]);
        return buf;
    }

    // 从外部内存封装（不拷贝，由调用方保证生命周期长于本对象）
    // 可用于包装 cv::Mat、camera SDK buffer 等已分配内存
    static std::shared_ptr<ImageBuffer> Wrap(
        uint8_t* raw, int w, int h, int c, int elem_size = 1, int step = 0)
    {
        auto buf      = std::make_shared<ImageBuffer>();
        buf->width    = w;
        buf->height   = h;
        buf->channels = c;
        buf->elem_size = elem_size;
        buf->step     = (step > 0) ? step : w * c * elem_size;
        // 空 deleter：内存由外部管理
        buf->data     = std::shared_ptr<uint8_t[]>(raw, [](uint8_t*) {});
        return buf;
    }
};

// ---------------------------------------------------------------------------
// AlgorithmValue — 类型擦除值容器，用于跨插件安全传递
//
// 以 shared_ptr<void> 持有任意类型数据，配合 typeid 字符串做类型校验。
// 跨 DLL 传递时无 std::any 的 RTTI 不稳定问题（同进程、同编译器下 name() 一致）。
// ---------------------------------------------------------------------------
class AlgorithmValue
{
public:
    AlgorithmValue() = default;

    // 从任意值构造（右值优先，减少拷贝）
    template <typename T>
    explicit AlgorithmValue(T&& val)
        : data_(std::make_shared<std::decay_t<T>>(std::forward<T>(val)))
        , type_name_(typeid(std::decay_t<T>).name())
    {}

    bool        Valid()    const noexcept { return data_ != nullptr; }
    const std::string& TypeName() const noexcept { return type_name_; }

    // 带类型安全检查的取值（失败返回 nullptr，不抛异常）
    template <typename T>
    T* TryGet() noexcept
    {
        return (type_name_ == typeid(T).name())
               ? static_cast<T*>(data_.get())
               : nullptr;
    }

    template <typename T>
    const T* TryGet() const noexcept
    {
        return (type_name_ == typeid(T).name())
               ? static_cast<const T*>(data_.get())
               : nullptr;
    }

private:
    std::shared_ptr<void> data_;
    std::string           type_name_;
};

// ---------------------------------------------------------------------------
// AlgorithmPacket — 算法 I/O 参数包
//
// 以 key→AlgorithmValue 的哈希表存储任意参数，提供图像零拷贝快捷接口。
// 整个对象以 shared_ptr 传递时自身也不拷贝数据。
// ---------------------------------------------------------------------------
class AlgorithmPacket
{
public:
    AlgorithmPacket() = default;
    AlgorithmPacket(const AlgorithmPacket &)            = default;
    AlgorithmPacket &operator=(const AlgorithmPacket &) = default;

    // --- 通用存取 ---

    // 设置任意类型参数（移动或拷贝）
    template <typename T>
    void Set(const std::string& key, T&& val)
    {
        entries_[key] = AlgorithmValue(std::forward<T>(val));
    }

    // 取任意类型参数指针（类型不匹配或 key 不存在返回 nullptr）
    template <typename T>
    T* Get(const std::string& key)
    {
        auto it = entries_.find(key);
        return (it != entries_.end()) ? it->second.TryGet<T>() : nullptr;
    }

    template <typename T>
    const T* Get(const std::string& key) const
    {
        auto it = entries_.find(key);
        return (it != entries_.end()) ? it->second.TryGet<T>() : nullptr;
    }

    // --- 图像快捷接口（shared_ptr 传递，零拷贝）---

    void SetImage(const std::string& key, std::shared_ptr<ImageBuffer> img)
    {
        // 存储 shared_ptr<ImageBuffer>，避免再套一层 make_shared
        entries_[key] = AlgorithmValue(std::move(img));
    }

    std::shared_ptr<ImageBuffer> GetImage(const std::string& key) const
    {
        auto it = entries_.find(key);
        if (it == entries_.end()) return nullptr;
        auto* ptr = it->second.TryGet<std::shared_ptr<ImageBuffer>>();
        return ptr ? *ptr : nullptr;
    }

    // --- 查询与管理 ---

    bool Has(const std::string& key) const
    {
        return entries_.find(key) != entries_.end();
    }

    void Remove(const std::string& key) { entries_.erase(key); }

    void Clear() { entries_.clear(); }

    std::vector<std::string> Keys() const
    {
        std::vector<std::string> ks;
        ks.reserve(entries_.size());
        for (const auto& kv : entries_)
            ks.push_back(kv.first);
        return ks;
    }

    std::size_t Size() const noexcept { return entries_.size(); }

private:
    std::unordered_map<std::string, AlgorithmValue> entries_;
};

// ---------------------------------------------------------------------------
// AlgorithmExecuteData — 算法执行上下文
//
// 一次 Execute 调用的完整 I/O 载体，以引用传入算法，避免整体拷贝。
// ---------------------------------------------------------------------------
struct AlgorithmExecuteData
{
    AlgorithmPacket inputs;   // 调用方填充后传入
    AlgorithmPacket outputs;  // 算法写入，调用方读取
};

}  // namespace algorithm
}  // namespace common
