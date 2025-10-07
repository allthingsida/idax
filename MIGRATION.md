# Migration Guide

**idax has been refactored with modern C++20, proper namespacing, and CMake integration.**

This guide helps you update your code to work with the new structure.

**Migration time:** 15-30 minutes for typical plugin

---

## What Changed

| Aspect | Before | After |
|--------|--------|-------|
| **Headers** | `<xpro.hpp>` | `<idacpp/core/core.hpp>` |
| **Namespace** | Global | `idacpp::core`, `idacpp::kernwin`, etc. |
| **Build** | Manual include | CMake target `idacpp::idacpp` |
| **Structure** | Single headers | Organized modules |
| **C++ Standard** | C++11/14 | C++20 |

**Good news:** All APIs are unchanged. Only headers and namespaces changed.

---

## Quick Migration

### 1. Update CMakeLists.txt

```cmake
# Before
include_directories(external/idax)

# After
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(external/idax)
target_link_libraries(your_plugin PRIVATE idacpp::idacpp)
```

### 2. Update Includes

```cpp
// Before
#include <xpro.hpp>
#include <xkernwin.hpp>
#include <xhexrays.hpp>
#include <xexpr.hpp>
#include <xcallbacks.hpp>

// After - Option 1: All modules
#include <idacpp/idacpp.hpp>

// After - Option 2: Specific modules
#include <idacpp/core/core.hpp>
#include <idacpp/kernwin/kernwin.hpp>
#include <idacpp/hexrays/hexrays.hpp>
#include <idacpp/expr/expr.hpp>
#include <idacpp/callbacks/callbacks.hpp>
```

### 3. Add Namespace Declarations

```cpp
// Add at top of file
using namespace idacpp::core;
using namespace idacpp::kernwin;
using namespace idacpp::hexrays;
using namespace idacpp::callbacks;
using idacpp::expr::pylang;
```

### 4. Fix Class-Scope Templates

```cpp
// Before
class MyPlugin {
    using core::objcontainer_t;
    objcontainer_t<T> container;
};

// After
class MyPlugin {
    idacpp::core::objcontainer_t<T> container;  // Fully qualified
};
```

---

## Module Reference

| Old Header | New Header | Namespace |
|------------|------------|-----------|
| `xpro.hpp` | `idacpp/core/core.hpp` | `idacpp::core` |
| `xkernwin.hpp` | `idacpp/kernwin/kernwin.hpp` | `idacpp::kernwin` |
| `xhexrays.hpp` | `idacpp/hexrays/hexrays.hpp` | `idacpp::hexrays` |
| `xexpr.hpp` | `idacpp/expr/expr.hpp` | `idacpp::expr` |
| `xcallbacks.hpp` | `idacpp/callbacks/callbacks.hpp` | `idacpp::callbacks` |

