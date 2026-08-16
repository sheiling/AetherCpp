# AOI Plugin Code Quality Guidelines

> **Version:** 1.0.0 · **Last Updated:** 2026-07-24

---

## Overview

This document outlines the code quality standards and best practices for the AOI plugin codebase. Following these guidelines ensures maintainability, readability, and consistency across the entire plugin.

---

## C++ Coding Standards

### Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes / Structs | PascalCase | `RecipeConfigViewModel`, `DefectInfo` |
| Functions / Methods | camelCase | `loadRecipeList()`, `setProperty()` |
| Member variables | `m_` prefix | `m_viewModel`, `m_properties` |
| Constants | `k` prefix | `kPreviewWinId`, `kDefaultTimeoutMs` |
| Macros | UPPER_SNAKE_CASE | `VIEWMODEL_PROPERTY`, `PARAM_DECLARE` |
| Enums | PascalCase (type) | `InspectionResult::Pass` |
| Namespaces | lowercase | `aoi::review`, `aoi::inspection` |
| File names | snake_case | `recipe_config_view_model.h` |

### Include Order

Maintain a consistent include order — each group separated by a blank line:

```cpp
// 1. Own header (in .cpp files)
#include "aoiplugin.h"

// 2. Same-plugin headers (from most specific to most general)
#include "service/inspection/detect/inspection_detect_service.h"
#include "ui/view/home/home_view.h"
#include "aoi_plugin_name.h"

// 3. Framework / common headers
#include "extensionsystem/pluginmanager.h"
#include "uibase/imainwindowservice.h"
#include "uibase/languagemanager.h"

// 4. Qt headers
#include <QApplication>
#include <QDebug>

// 5. Standard library
#include <memory>
#include <optional>
```

### Header File Best Practices

- Use `#pragma once` instead of traditional include guards
- Prefer forward declarations over includes when possible
- Keep headers minimal — only expose what's necessary
- Use `const&` for non-trivial parameters

```cpp
#pragma once

#include <QString>
#include <memory>

class QWidget;  // forward declaration

class RecipeService {
public:
    explicit RecipeService(QWidget *parent = nullptr);
    QString recipeName() const;
    void setRecipeName(const QString &name);  // const ref for non-trivial
};
```

### Code Organization per File

1. **Header (.h):** Class declaration → public API → protected → private members
2. **Implementation (.cpp):** Includes → anonymous namespace (helpers) → public methods → private methods

```cpp
// .cpp file structure
#include "header.h"

// ── Anonymous namespace for TU-local helpers ──
namespace {
    QString defaultRecipePath() { return ...; }
}

// ── Public API ──
bool MyClass::doSomething() { ... }

// ── Private helpers ──
void MyClass::validateInput() { ... }
```

---

## Documentation Standards

### File Header Comment

Every `.h` and `.cpp` file must have a Doxygen-style file header:

```cpp
/**
 * @file recipe_config_view_model.h
 * @brief ViewModel for the recipe configuration page
 *
 * Manages the state and business logic for recipe CRUD operations,
 * including preview management, camera control, and parameter validation.
 *
 * @see RecipeConfigView
 * @see RecipeDocument
 */
```

### Class Documentation

```cpp
/**
 * @brief Manages the recipe configuration workflow
 *
 * Coordinates between the RecipeRepository, camera service, and UI views.
 * Implements the MVVM ViewModel pattern for the recipe configuration page.
 *
 * ## Usage Example
 * @code
 * auto vm = new RecipeConfigViewModel("recipe_config", parent);
 * vm->loadRecipeList();
 * @endcode
 *
 * @see RecipeRepository
 * @see RecipeConfigView
 */
class RecipeConfigViewModel : public ViewModel { ... };
```

### Method Documentation

Document all public methods with `@brief`, `@param`, and `@return`:

```cpp
/**
 * @brief Load the recipe list from persistent storage
 *
 * Queries the RecipeRepository for all available recipes and updates
 * the internal list. Emits recipesChanged() on completion.
 *
 * @param forceRefresh If true, bypasses cache and reloads from disk
 * @return Number of recipes loaded, or -1 on error
 *
 * @see RecipeRepository::listRecipes()
 */
int loadRecipeList(bool forceRefresh = false);
```

### Inline Comments

Comment the **why**, not the **what**:

```cpp
// ❌ Bad: States the obvious
x = x + 1;  // increment x by 1

// ✅ Good: Explains the reasoning
// 使用 20ms 单次定时器而非自动重复，避免任务堆积
delayedInitializeTimer->setInterval(20);
```

---

## Qt6 Best Practices

### Signal/Slot Connections

```cpp
// ✅ Good: Modern function-pointer syntax (compile-time checked)
connect(sender, &Sender::signal, receiver, &Receiver::slot);

// ❌ Bad: String-based syntax (deprecated in Qt6, no compile-time check)
connect(sender, SIGNAL(signal()), receiver, SLOT(slot()));
```

### Container Usage

```cpp
// ✅ Good: Qt6 containers
QList<QString> list;           // QList is the universal sequential container
QHash<QString, int> map;        // Fast lookup
QSet<int> uniqueValues;         // Unique set

// ❌ Bad: Removed or deprecated in Qt6
QLinkedList<QString> list;      // Removed in Qt6 — use QList
QStringRef ref;                 // Removed — use QStringView
QVector<int> vec;               // QVector is now an alias for QList
```

### Memory Management

1. **QObject hierarchy:** Use Qt's parent-child ownership — children auto-delete with parent
2. **Smart pointers:** Use `std::unique_ptr` / `std::shared_ptr` for non-QObject ownership
3. **Avoid raw `new`/`delete`:** Prefer RAII wrappers

```cpp
// ✅ Good: QObject parent ownership
auto *button = new QPushButton("Click", parentWidget);

// ✅ Good: Smart pointer for non-QObject
auto service = std::make_unique<MyService>();

// ❌ Bad: Raw owning pointer
MyClass *obj = new MyClass();  // Who deletes it?
```

### Deprecated Qt5 APIs to Avoid

| Deprecated Qt5 API | Qt6 Replacement | Notes |
|-------------------|-----------------|-------|
| `QRegExp` | `QRegularExpression` | Different API, more powerful |
| `QTextCodec` | `QStringConverter` | New encoding API |
| `QDesktopWidget` | `QScreen` | Screen info via `QGuiApplication::screens()` |
| `QString::SkipEmptyParts` | `Qt::SkipEmptyParts` | Moved to Qt namespace |
| `QMatrix` | `QTransform` | 3x3 transform matrix |
| `QButtonGroup::buttonClicked(int)` | `QButtonGroup::idClicked(int)` | Signal renamed |

---

## MVVM Architecture Guidelines

### ViewModel Rules

1. **All public properties** must be declared via `VIEWMODEL_PROPERTY` macros
2. **Business logic** lives in ViewModel, never in View
3. **Views** are passive — they only bind and display

```cpp
// ViewModel: owns the state
class MyViewModel : public ViewModel {
    Q_OBJECT
    VIEWMODEL_PROPERTY(QString, userName, UserName)
    VIEWMODEL_PROPERTY(int, progress, Progress)

public:
    void loadData();  // business logic here
};

// View: only binds
class MyView : public PageWidget {
    void setupBindings() {
        bind(m_vm, "userName", m_nameEdit);
        bindOneWay(m_vm, "progress", m_progressBar);
    }
};
```

### Data Binding Patterns

- **Two-way:** `bind(vm, "name", lineEdit)` — ViewModel ↔ QLineEdit
- **One-way:** `bindOneWay(vm, "progress", progressBar)` — ViewModel → QWidget
- **Command:** `bindCommand(cmd, button)` — QPushButton → Command
- **Struct field:** `bindStructField<T>(vm, "address", getField, setField, widget)` — partial struct updates

---

## Testing Standards

### Unit Test Structure (Google Test)

```cpp
TEST(RecipeConfigTest, LoadRecipeListReturnsCorrectCount)
{
    // Arrange
    RecipeConfigViewModel vm("test");

    // Act
    vm.loadRecipeList();

    // Assert
    EXPECT_EQ(vm.recipes().size(), expectedCount);
}
```

### Test Coverage Goals

1. **Target:** 80%+ code coverage for service and repository layers
2. **Critical paths:** All public API methods must have tests
3. **Edge cases:** Test boundary conditions, empty states, and error paths
4. **Naming:** `TEST(ClassName, MethodName_Scenario_ExpectedResult)`

### Test Fixture Pattern

```cpp
class RecipeRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(m_tempDir.isValid());
        RecipeRepository::setStorageDirectory(m_tempDir.path());
    }

    void TearDown() override {
        // Cleanup is automatic via QTemporaryDir
    }

    QTemporaryDir m_tempDir;
};
```

---

## Performance Guidelines

| Principle | Practice |
|-----------|----------|
| **Avoid copies** | Use `const&` for large objects, `std::move` where appropriate |
| **Lazy loading** | Load data only when needed (`PageWidget::onLoad()`) |
| **Caching** | Cache expensive computations, invalidate on change |
| **Batch updates** | Use `beginBatchUpdate()` / `endBatchUpdate()` for bulk property changes |
| **Profiling** | Use Qt Creator's profiler for performance analysis |

---

## Git Commit Conventions

Use conventional commits with Chinese descriptions:

```
<type>(<scope>): <description>

feat(aoi): Add recipe export functionality
fix(aoi): Fix crash when loading empty recipe list
docs(aoi): Add recipe model architecture documentation
test(aoi): Add unit tests for RecipeRepository
refactor(aoi): Simplify FOV planner coordinate system
```

### Types

| Type | Usage |
|------|-------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation only |
| `test` | Adding or modifying tests |
| `refactor` | Code restructuring (no functional change) |
| `perf` | Performance improvement |

---

## Code Review Checklist

- [ ] Code follows naming conventions
- [ ] Documentation is complete and accurate (Doxygen)
- [ ] Tests cover new functionality
- [ ] No memory leaks (RAII, parent ownership, smart pointers)
- [ ] Qt6 best practices followed (no deprecated APIs)
- [ ] Include order is correct
- [ ] No magic numbers — use named constants
- [ ] Error handling is appropriate
- [ ] Thread safety is considered
- [ ] No unnecessary dependencies or includes