# Migration Guide

**idax headers have been reorganized into modules with proper namespacing.**

This guide shows you how to update your code.

**Time needed:** 15-30 minutes for typical plugin

---

## What Changed

| Aspect | Old | New |
|--------|-----|-----|
| **Headers** | `<xpro.hpp>` | `<idacpp/core/core.hpp>` |
| **Namespace** | Global | `idacpp::core`, `idacpp::kernwin`, etc. |
| **Build** | Manual include | CMake target `idacpp::idacpp` |
| **C++ Standard** | C++11/14 | C++20 |

**All APIs are unchanged** - only headers and namespaces changed.

---

## Update Your Code

### 1. CMakeLists.txt

```cmake
# Old
include_directories(external/idax)

# New
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(external/idax)
target_link_libraries(your_plugin PRIVATE idacpp::idacpp)
```

### 2. Includes

```cpp
// Old
#include <xpro.hpp>
#include <xkernwin.hpp>
#include <xhexrays.hpp>
#include <xexpr.hpp>
#include <xcallbacks.hpp>

// New - All modules
#include <idacpp/idacpp.hpp>

// Or specific modules
#include <idacpp/core/core.hpp>
#include <idacpp/kernwin/kernwin.hpp>
#include <idacpp/hexrays/hexrays.hpp>
#include <idacpp/expr/expr.hpp>
#include <idacpp/callbacks/callbacks.hpp>
```

### 3. Namespaces

```cpp
// Add at top of file
using namespace idacpp::core;
using namespace idacpp::kernwin;
using namespace idacpp::hexrays;
using namespace idacpp::callbacks;
using idacpp::expr::pylang;
```

### 4. Class Members

```cpp
// Old
class MyPlugin {
    using core::objcontainer_t;
    objcontainer_t<T> container;
};

// New
class MyPlugin {
    idacpp::core::objcontainer_t<T> container;  // Fully qualified
};
```

---

## Header Mapping

| Old | New | Namespace |
|-----|-----|-----------|
| `xpro.hpp` | `idacpp/core/core.hpp` | `idacpp::core` |
| `xkernwin.hpp` | `idacpp/kernwin/kernwin.hpp` | `idacpp::kernwin` |
| `xhexrays.hpp` | `idacpp/hexrays/hexrays.hpp` | `idacpp::hexrays` |
| `xexpr.hpp` | `idacpp/expr/expr.hpp` | `idacpp::expr` |
| `xcallbacks.hpp` | `idacpp/callbacks/callbacks.hpp` | `idacpp::callbacks` |

---

## Example

### Before

```cpp
// CMakeLists.txt
include_directories(external/idax)
ida_add_plugin(myplugin SOURCES myplugin.cpp)

// myplugin.cpp
#include <xpro.hpp>
#include <xkernwin.hpp>

class my_plugin_t : public plugmod_t {
    objcontainer_t<handler_t> handlers;
    action_manager_t actions;
};
```

### After

```cpp
// CMakeLists.txt
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(external/idax)
ida_add_plugin(myplugin SOURCES myplugin.cpp)
target_link_libraries(myplugin PRIVATE idacpp::idacpp)

// myplugin.cpp
#include <idacpp/idacpp.hpp>
using namespace idacpp::core;
using namespace idacpp::kernwin;

class my_plugin_t : public plugmod_t {
    core::objcontainer_t<handler_t> handlers;
    action_manager_t actions;
};
```

---

## Common Issues

**"No such file: xpro.hpp"**
```cpp
#include <idacpp/core/core.hpp>
```

**"objcontainer_t not declared"**
```cpp
using namespace idacpp::core;
```

**"Cannot use template 'using' in class"**
```cpp
class MyPlugin {
    idacpp::core::objcontainer_t<T> container;
};
```

---

## Checklist

- [ ] Set C++ standard to 20
- [ ] Add `add_subdirectory(external/idax)`
- [ ] Add `target_link_libraries(... idacpp::idacpp)`
- [ ] Update includes: `<x*.hpp>` → `<idacpp/...>`
- [ ] Add namespace using declarations
- [ ] Fix class-scope template using (if any)
- [ ] Test compilation
- [ ] Test plugin loads

---

## Old Structure

The old flat structure (`x*.hpp` files) is preserved in the `legacy` branch.

```bash
git checkout legacy  # Access old structure if needed
```

---

**Questions?** See the `examples/` directory for working code.
