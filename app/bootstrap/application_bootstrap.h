#pragma once

#include "bootstrap_config.h"

#include <memory>
#include <vector>

class IBootstrapModule;

/// 编排 QApplication、PluginManager 与各 IBootstrapModule 的启动与关闭顺序。
class ApplicationBootstrap {
public:
    explicit ApplicationBootstrap(BootstrapConfig config = {});
    ~ApplicationBootstrap();

    void addModule(std::unique_ptr<IBootstrapModule> module);

    /// 创建 QApplication，依次执行各阶段；成功时返回 exec() 的退出码。
    int run(int &argc, char **argv);

private:
    void sortModules();

    BootstrapConfig config_;
    std::vector<std::unique_ptr<IBootstrapModule>> modules_;
};
