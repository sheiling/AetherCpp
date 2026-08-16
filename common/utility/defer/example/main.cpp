#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

// 包含 defer 头文件
#include "utility/defer/include/Defer.h"

using namespace utility;

// =============================================================================
// 示例 1: 文件资源管理
// =============================================================================
void example_file_management() {
    std::cout << "\n=== 示例 1: 文件资源管理 ===" << std::endl;
    
    std::ofstream file("test_defer.txt");
    DEFER([&]() {
        if (file.is_open()) {
            file.close();
            std::cout << "文件已关闭" << std::endl;
        }
    });
    
    file << "Hello, Defer!" << std::endl;
    std::cout << "文件写入完成" << std::endl;
    // 函数退出时自动关闭文件
}

// =============================================================================
// 示例 2: 动态内存管理
// =============================================================================
void example_memory_management() {
    std::cout << "\n=== 示例 2: 动态内存管理 ===" << std::endl;
    
    int* ptr = new int(42);
    DEFER([&]() {
        delete ptr;
        ptr = nullptr;
        std::cout << "内存已释放" << std::endl;
    });
    
    std::cout << "值: " << *ptr << std::endl;
    // 函数退出时自动释放内存
}

// =============================================================================
// 示例 3: 异常安全
// =============================================================================
void example_exception_safety() {
    std::cout << "\n=== 示例 3: 异常安全 ===" << std::endl;
    
    std::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    
    DEFER([&]() {
        std::cout << "清理操作执行，vector 大小: " << vec.size() << std::endl;
    });
    
    try {
        std::cout << "可能抛出异常的操作..." << std::endl;
        // 即使这里抛出异常，defer 也会执行
        throw std::runtime_error("测试异常");
    } catch (...) {
        std::cout << "捕获到异常" << std::endl;
    }
    // defer 在 catch 块之后执行
}

// =============================================================================
// 示例 4: 多个 defer（LIFO 顺序）
// =============================================================================
void example_multiple_defers() {
    std::cout << "\n=== 示例 4: 多个 defer（LIFO 顺序）===" << std::endl;
    
    DEFER([]() { std::cout << "Defer 3: 最后执行" << std::endl; });
    DEFER([]() { std::cout << "Defer 2: 第二个执行" << std::endl; });
    DEFER([]() { std::cout << "Defer 1: 第一个执行" << std::endl; });
    
    std::cout << "函数体执行中..." << std::endl;
    // 退出时按 LIFO 顺序执行（类似 Go 的 defer）
}

// =============================================================================
// 示例 5: 条件取消 defer
// =============================================================================
void example_cancel_defer() {
    std::cout << "\n=== 示例 5: 条件取消 defer ===" << std::endl;
    
    bool need_cleanup = true;
    DEFER_NAMED(cleanup, [&]() {
        if (need_cleanup) {
            std::cout << "执行清理操作" << std::endl;
        } else {
            std::cout << "清理操作已取消" << std::endl;
        }
    });
    
    std::cout << "某些操作..." << std::endl;
    
    // 根据条件取消
    need_cleanup = false;
    cleanup.cancel();
    
    std::cout << "函数退出" << std::endl;
}

// =============================================================================
// 示例 6: 提前执行 defer
// =============================================================================
void example_early_execute() {
    std::cout << "\n=== 示例 6: 提前执行 defer ===" << std::endl;
    
    DEFER_NAMED(cleanup, []() {
        std::cout << "清理操作执行" << std::endl;
    });
    
    std::cout << "某些操作..." << std::endl;
    
    // 提前执行清理
    cleanup.execute();
    std::cout << "清理操作已提前执行" << std::endl;
    
    // 函数退出时不会再执行
    std::cout << "函数退出" << std::endl;
}

// =============================================================================
// 示例 7: 使用函数指针
// =============================================================================
void cleanup_function() {
    std::cout << "使用函数指针的清理操作" << std::endl;
}

void example_function_pointer() {
    std::cout << "\n=== 示例 7: 使用函数指针 ===" << std::endl;
    
    DEFER(cleanup_function);
    
    std::cout << "函数体执行中..." << std::endl;
    // 退出时调用 cleanup_function
}

// =============================================================================
// 示例 8: 捕获局部变量
// =============================================================================
void example_capture_variables() {
    std::cout << "\n=== 示例 8: 捕获局部变量 ===" << std::endl;
    
    int count = 0;
    std::string message = "初始消息";
    
    DEFER([&]() {
        std::cout << "最终 count: " << count << std::endl;
        std::cout << "最终 message: " << message << std::endl;
    });
    
    count = 100;
    message = "修改后的消息";
    std::cout << "修改了变量" << std::endl;
    // defer 会捕获最终的值
}

// =============================================================================
// 主函数
// =============================================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Defer 功能演示" << std::endl;
    std::cout << "========================================" << std::endl;
    
    example_file_management();
    example_memory_management();
    example_exception_safety();
    example_multiple_defers();
    example_cancel_defer();
    example_early_execute();
    example_function_pointer();
    example_capture_variables();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "所有示例执行完成" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
