# Defer - 类似 Go 语言的延迟执行功能

一个简洁易用的 C++ header-only 库，实现类似 Go 语言的 `defer` 功能，用于确保资源清理和异常安全。

## 特性

- ✅ **Header-only**：单头文件，无需编译，直接包含即可使用
- ✅ **简洁易用**：提供 `DEFER` 宏，使用方式类似 Go 语言
- ✅ **异常安全**：无论函数正常返回还是抛出异常，都会执行清理操作
- ✅ **LIFO 顺序**：多个 defer 按后进先出顺序执行（与 Go 一致）
- ✅ **灵活控制**：支持取消和提前执行
- ✅ **C++17 兼容**：使用现代 C++ 特性，代码简洁高效

## 快速开始

### 基本用法

```cpp
#include "utility/defer/include/Defer.h"

void example() {
    FILE* fp = fopen("test.txt", "r");
    DEFER([&]() { 
        if (fp) fclose(fp); 
    });
    
    // 无论函数如何退出，fp 都会被关闭
    if (!fp) return;
    // ... 使用 fp ...
}
```

### 文件资源管理

```cpp
#include <fstream>
#include "utility/defer/include/Defer.h"

void write_file() {
    std::ofstream file("output.txt");
    DEFER([&]() {
        if (file.is_open()) {
            file.close();
        }
    });
    
    file << "Hello, World!" << std::endl;
    // 函数退出时自动关闭文件
}
```

### 动态内存管理

```cpp
#include "utility/defer/include/Defer.h"

void memory_example() {
    int* ptr = new int(42);
    DEFER([&]() {
        delete ptr;
        ptr = nullptr;
    });
    
    // 使用 ptr...
    // 函数退出时自动释放内存
}
```

### 异常安全

```cpp
#include "utility/defer/include/Defer.h"
#include <vector>

void exception_safe() {
    std::vector<int> vec;
    vec.push_back(1);
    
    DEFER([&]() {
        std::cout << "清理操作执行" << std::endl;
    });
    
    // 即使这里抛出异常，defer 也会执行
    throw std::runtime_error("错误");
}
```

### 多个 Defer（LIFO 顺序）

```cpp
#include "utility/defer/include/Defer.h"

void multiple_defers() {
    DEFER([]() { std::cout << "第三个执行" << std::endl; });
    DEFER([]() { std::cout << "第二个执行" << std::endl; });
    DEFER([]() { std::cout << "第一个执行" << std::endl; });
    
    // 退出时按 LIFO 顺序执行（后进先出）
}
```

### 取消 Defer

```cpp
#include "utility/defer/include/Defer.h"

void cancel_example() {
    DEFER_NAMED(cleanup, []() {
        std::cout << "清理操作" << std::endl;
    });
    
    // 根据条件取消
    if (some_condition) {
        cleanup.cancel();
    }
    
    // 函数退出时不会执行清理
}
```

### 提前执行 Defer

```cpp
#include "utility/defer/include/Defer.h"

void early_execute() {
    DEFER_NAMED(cleanup, []() {
        std::cout << "清理操作" << std::endl;
    });
    
    // 提前执行清理
    cleanup.execute();
    
    // 函数退出时不会再执行
}
```

### 使用函数指针

```cpp
#include "utility/defer/include/Defer.h"

void cleanup_function() {
    std::cout << "清理操作" << std::endl;
}

void function_pointer_example() {
    DEFER(cleanup_function);
    // 函数退出时调用 cleanup_function
}
```

## API 参考

### `DEFER(expr)`

便捷宏，创建匿名 Defer 对象。

**参数：**
- `expr`: 可调用对象表达式（通常是 lambda）

**示例：**
```cpp
DEFER([]() { cleanup(); });
DEFER([&]() { resource.release(); });
```

### `DEFER_NAMED(name, expr)`

带名称的 defer 宏，可以手动控制执行。

**参数：**
- `name`: 变量名称
- `expr`: 可调用对象表达式

**示例：**
```cpp
DEFER_NAMED(cleanup, []() { cleanup(); });
cleanup.execute();  // 提前执行
cleanup.cancel();   // 取消执行
```

### `Defer` 类方法

#### `void cancel()`

取消延迟执行。调用后，析构时不会执行保存的操作。

#### `void execute()`

立即执行并取消延迟执行。立即执行保存的操作，之后析构时不再执行。

## 实现原理

使用 RAII (Resource Acquisition Is Initialization) 模式实现：

1. **构造函数**：保存要延迟执行的可调用对象
2. **析构函数**：在对象生命周期结束时执行保存的操作
3. **移动语义**：支持移动构造和移动赋值，禁止拷贝以避免重复执行

## 编译要求

- C++17 或更高版本
- 标准库支持（`<functional>`, `<utility>`）

## 运行示例

示例代码位于 `example/main.cpp`，包含多个使用场景的演示。

编译运行：
```bash
cd common/utility/defer/example
g++ -std=c++17 main.cpp -o defer_example
./defer_example
```

## 注意事项

1. **变量捕获**：使用 lambda 时注意捕获方式（`[&]` 引用捕获或 `[=]` 值捕获）
2. **执行顺序**：多个 defer 按 LIFO（后进先出）顺序执行
3. **异常安全**：defer 在异常抛出时也会执行，确保资源清理
4. **性能**：使用 `std::function` 可能有一定的性能开销，但在大多数场景下可以忽略

## 许可证

本项目遵循项目整体许可证。
