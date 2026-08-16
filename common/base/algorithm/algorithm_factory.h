#ifndef COMMON_ALGORITHM_ALGORITHM_FACTORY_H_
#define COMMON_ALGORITHM_ALGORITHM_FACTORY_H_

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <common/base/algorithm/algorithm_global.h>
#include <common/base/algorithm/algorithm_base.h>

namespace common
{
namespace algorithm
{

class AlgorithmFactory
{
public:
    using Creator = std::function<std::unique_ptr<AlgorithmBase>()>;

    // 单例必须在 base 库中定义并导出，避免各插件 DLL 各自持有一份 creators_。
    static ALGORITHM_EXPORT AlgorithmFactory& Instance();

    bool Register(const std::string& name, Creator creator)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return creators_.emplace(name, std::move(creator)).second;
    }

    std::unique_ptr<AlgorithmBase> Create(const std::string& name) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto iter = creators_.find(name);
        if (iter == creators_.end())
        {
            return nullptr;
        }
        return iter->second();
    }

    bool Contains(const std::string& name) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return creators_.find(name) != creators_.end();
    }

    std::vector<std::string> List() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> names;
        names.reserve(creators_.size());
        for (const auto& item : creators_)
        {
            names.push_back(item.first);
        }
        return names;
    }

private:
    AlgorithmFactory() = default;

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Creator> creators_;
};

template <typename TAlgorithm>
class AlgorithmRegistrar
{
public:
    explicit AlgorithmRegistrar(const std::string& name)
    {
        AlgorithmFactory::Instance().Register(name, []() {
            return std::unique_ptr<AlgorithmBase>(new TAlgorithm());
        });
    }
};

#define ALGORITHM_REGISTER_CONCAT_IMPL(X, Y) X##Y
#define ALGORITHM_REGISTER_CONCAT(X, Y) ALGORITHM_REGISTER_CONCAT_IMPL(X, Y)

#define REGISTER_ALGORITHM(NAME, TYPE)                                                \
    namespace                                                                          \
    {                                                                                  \
        const ::common::algorithm::AlgorithmRegistrar<TYPE>                           \
            ALGORITHM_REGISTER_CONCAT(g_algorithm_registrar_, __LINE__)(NAME);        \
    }

}  // namespace algorithm
}  // namespace common

#endif  // COMMON_ALGORITHM_ALGORITHM_FACTORY_H_
