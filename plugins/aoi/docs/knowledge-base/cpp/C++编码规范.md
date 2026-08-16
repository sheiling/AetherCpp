## C++编码全规范（安全/可扩展/可维护）
### 1. 核心基础编码规范
#### 1.1 命名规范
- 类型名（类/结构体/枚举）：大驼峰，如**DataProcessor**、**OrderStatus**
- 函数/方法名：小驼峰，如**processData()**、**getOrderId()**
- 变量名：小驼峰，如**userName**、**totalAmount**
- 常量名：全大写+下划线，如**MAX_CONNECTIONS**、**DEFAULT_TIMEOUT**
- 命名空间：小写+下划线，如**common_utils**、**business_logic**
- 私有成员变量：后缀加下划线，如**data_buffer_**、**is_running_**

#### 1.2 格式规范
- 缩进：4个空格（禁止tab）
- 行宽：单行不超过120个字符
- 大括号：换行对齐（函数/类），单行语句可省略（但不推荐）
- 空格：运算符两侧加空格，如`a = b + c`；逗号后加空格，如`func(a, b)`

#### 1.3 注释规范
- 类/函数：使用Doxygen风格注释，包含功能、参数、返回值、异常
- 关键逻辑：单行注释`// [核心逻辑] 处理空指针场景`
- 复杂代码块：多行注释`/* 多线程竞争场景下的锁粒度控制，需保证：1. 锁仅包裹临界区 2. 避免锁内阻塞 */`
- 废弃代码：标注[废弃]，如`// [废弃] 该接口已被processV2替代，v1.5后删除`

### 2. 安全编码规范
#### 2.1 内存安全
- 禁止使用裸指针管理资源，优先使用**std::unique_ptr**/**std::shared_ptr** [C++11]
- 禁止直接调用**delete**/**free**，通过RAII机制自动释放
- 数组使用**std::vector**替代原生数组，避免越界，如`std::vector<int> arr(10)`而非`int arr[10]`
- 指针判空：所有指针使用前必须检查是否为**nullptr**，禁止直接解引用
- 禁止内存泄漏：使用工具（Valgrind/AddressSanitizer）检测，核心模块[必填]

#### 2.2 类型安全
- 禁止隐式类型转换，使用**static_cast**/**reinterpret_cast**显式转换（注明原因）
- 枚举使用强类型枚举`enum class`，如`enum class ErrorCode { Success = 0, Fail = 1 }`
- 禁止使用void*（除非底层接口必须，需标注[底层兼容]）

#### 2.3 线程安全
- 共享资源必须加锁，优先使用**std::lock_guard**/**std::unique_lock**而非手动lock/unlock
- 禁止在锁内调用阻塞函数（如sleep/网络IO），减小锁粒度
- 多线程变量使用**std::atomic**修饰，如`std::atomic<int> counter = 0`
- 避免死锁：统一多锁加锁顺序，或使用**std::lock()** 一次性加锁

#### 2.4 输入输出安全
- 禁止使用`scanf`/`gets`，优先使用`std::cin`+边界检查，或**fmt::format**
- 文件操作必须检查返回值，如`if (!file.is_open()) { throw FileOpenError(); }`
- 网络数据接收需校验长度/格式，避免缓冲区溢出

### 3. 可扩展编码规范
#### 3.1 架构设计
- 单一职责：一个类/函数只做一件事，如`FileReader`仅负责文件读取，不包含数据处理
- 开闭原则：对扩展开放、对修改关闭，优先通过继承/接口扩展，而非修改原有代码
- 接口隔离：拆分大接口为小接口，如将`DataInterface`拆分为`DataReader`/`DataWriter`

#### 3.2 模块化设计
- 功能按模块拆分，每个模块对应独立的头文件/源文件，如`network/`、`utils/`
- 头文件保护：使用`#pragma once`或宏定义保护，避免重复包含
- 命名空间隔离：不同模块使用不同命名空间，如`namespace network::tcp`

#### 3.3 接口设计
- 核心接口使用纯虚类定义，如`class IDataProcessor { public: virtual void process() = 0; virtual ~IDataProcessor() = default; }`
- 接口版本管理：新增功能时保留旧接口，标注[废弃]，如`// [废弃] process() v1.0，推荐使用processV2()`
- 避免接口耦合：参数优先使用值/智能指针，而非具体类引用

### 4. 可维护编码规范
#### 4.1 代码可读性
- 函数长度：单个函数不超过50行，超过则拆分（如将初始化逻辑拆为`init()`）
- 避免魔法值：使用常量替代，如`const int TIMEOUT_MS = 5000`而非直接写5000
- 逻辑扁平化：减少if/else嵌套（建议不超过3层），优先使用提前return

#### 4.2 错误处理
- 异常：核心模块使用自定义异常类（继承`std::exception`），如`class FileError : public std::exception { ... }`
- 返回值：非核心模块使用错误码（强类型枚举），如`ErrorCode func()`
- 错误日志：关键错误必须输出日志，包含**错误码**/**调用栈**/**上下文**

#### 4.3 测试友好
- 核心函数/类设计为可测试，避免硬编码依赖（如通过构造函数注入依赖）
- 单元测试覆盖核心逻辑，标注[测试覆盖]，如`// [测试覆盖] 覆盖空输入、正常输入、超长输入场景`
- 预留调试接口：核心类保留`dump()`方法，输出内部状态（仅调试模式启用）