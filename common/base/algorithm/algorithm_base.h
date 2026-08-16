#ifndef COMMON_ALGORITHM_ALGORITHM_BASE_H_
#define COMMON_ALGORITHM_ALGORITHM_BASE_H_

#include <string>

#include <common/base/algorithm/algorithm_global.h>
#include <common/base/algorithm/algorithm_packet.h>
#include <common/utility/errors/include/error.h>

namespace common
{
namespace algorithm
{

// ---------------------------------------------------------------------------
// AlgorithmBase — 算法插件基类
//
// 所有具体算法必须继承本类并实现 Name() 与 Execute()。
// Initialize / Uninitialize 可选实现，用于资源加载与释放。
// Version 可选重写，用于运行时版本查询。
//
// 生命周期：
//   AlgorithmFactory::Create() → Initialize() → Execute()... → Uninitialize()
// ---------------------------------------------------------------------------
class ALGORITHM_EXPORT AlgorithmBase
{
public:
    virtual ~AlgorithmBase() = default;

    // 算法唯一标识名，与工厂注册 key 保持一致
    virtual std::string Name() const = 0;

    // 算法版本号，格式建议 "major.minor.patch"
    virtual std::string Version() const;

    // 初始化：加载模型、申请资源等耗时操作放在此处
    // 返回 nullptr 表示成功，非空 ErrorPtr 表示失败
    virtual common::errors::ErrorPtr Initialize();

    // 反初始化：释放 Initialize 中申请的资源
    virtual void Uninitialize();

    // 核心执行接口：
    //   data.inputs  — 调用方填充后传入
    //   data.outputs — 算法写入结果，调用方读取
    // 返回 nullptr 表示成功，非空 ErrorPtr 表示执行失败
    virtual common::errors::ErrorPtr Execute(AlgorithmExecuteData& data) = 0;
};

}  // namespace algorithm
}  // namespace common

#endif  // COMMON_ALGORITHM_ALGORITHM_BASE_H_
