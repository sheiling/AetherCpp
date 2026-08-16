#include <string>

#include <common/base/algorithm/algorithm_global.h>
#include <common/base/algorithm/algorithm_base.h>
#include <common/base/algorithm/algorithm_factory.h>

namespace common
{
namespace algorithm
{

std::string AlgorithmBase::Version() const
{
    return "1.0.0";
}

common::errors::ErrorPtr AlgorithmBase::Initialize()
{
    return nullptr;
}

void AlgorithmBase::Uninitialize()
{}

AlgorithmFactory& AlgorithmFactory::Instance()
{
    static AlgorithmFactory instance;
    return instance;
}

extern "C" ALGORITHM_EXPORT const char* AlgorithmModuleName()
{
    return "common_algorithm";
}

}  // namespace algorithm
}  // namespace common

