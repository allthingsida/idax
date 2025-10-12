# idacpp - Modern C++ Extensions for IDA SDK

A modern, namespace-organized C++ library providing high-level utilities and abstractions for IDA Pro plugin development.

Modern C++20 library with proper namespacing and ida-cmake integration.

## Features

- **Modern C++20** - Leverages modern C++ features and best practices
- **Namespace Organized** - Clean `idacpp::module` namespace structure
- **Header-Only** - Easy integration, no linking required (with optional compiled components)
- **ida-cmake Integration** - Seamless integration with IDA plugin build workflow
- **Type-Safe Utilities** - RAII wrappers and smart abstractions over IDA SDK

## Modules

### Core (`idacpp::core`)
Low-level utilities and container types:
- `objcontainer_t` - RAII object container with automatic lifetime management

### Kernwin (`idacpp::kernwin`)
UI and action management utilities:
- `action_manager_t` - Simplified IDA action creation and management
- `function_action_handler_t` - Function object-based action handlers
- `IDAICONS` - Named constants for IDA's built-in icons
- Action helper macros for lambda-based handlers

### Hexrays (`idacpp::hexrays`)
Decompiler utilities:
- `ctreeparent_visitor_t` - Enhanced ctree visitor with parent tracking
- Selection and range utilities for decompiler views
- Default action state handlers for Hexrays widgets

### Expr (`idacpp::expr`)
Expression evaluation utilities

### Callbacks (`idacpp::callbacks`)
Callback management utilities

## Requirements

- **IDA SDK** with ida-cmake
- **C++20 compiler** (MSVC 2019+, GCC 10+, Clang 10+)
- **CMake 3.27+**

## Quick Start

### 1. Clone into your plugin project

```bash
cd your_plugin_project
git submodule add https://github.com/allthingsida/idax.git
```

### 2. Add to your CMakeLists.txt

```cmake
add_subdirectory(external/idacpp)

ida_add_plugin(your_plugin
    SOURCES your_plugin.cpp
)

target_link_libraries(your_plugin PRIVATE idacpp::idacpp)
```

### 3. Use in your code

```cpp
#include <idacpp/idacpp.hpp>  // All modules
// or
#include <idacpp/kernwin/kernwin.hpp>  // Specific module

using namespace idacpp::kernwin;

// Use action manager
action_manager_t actions;
actions.add_action(
    AMAHF_NONE,
    "my_action",
    "My Action",
    nullptr,
    FO_ACTION_UPDATE([], {
        return AST_ENABLE_ALWAYS;
    }),
    FO_ACTION_ACTIVATE([](action_activation_ctx_t* ctx) {
        msg("Hello from idacpp!\n");
        return 1;
    })
);
```

## Usage Examples

### Action Management

```cpp
#include <idacpp/kernwin/kernwin.hpp>

using namespace idacpp::kernwin;

action_manager_t mgr;

// Create action with lambda handlers
mgr.add_action(
    AMAHF_NONE,
    "analyze_function",
    "Analyze Function",
    nullptr,
    FO_ACTION_UPDATE([], {
        return get_screen_ea() != BADADDR ? AST_ENABLE : AST_DISABLE;
    }),
    FO_ACTION_ACTIVATE([](action_activation_ctx_t* ctx) {
        ea_t ea = get_screen_ea();
        msg("Analyzing function at %a\n", ea);
        return 1;
    })
);
```

### Hexrays Visitor with Parent Tracking

```cpp
#include <idacpp/hexrays/hexrays.hpp>

using namespace idacpp::hexrays;

ctreeparent_visitor_t visitor;
visitor.apply_to(*cfunc, nullptr);

// Find parent of an expression
const citem_t* parent = visitor.parent_of(expr);

// Check ancestry
if (visitor.is_ancestor_of(parent, child)) {
    // ...
}
```

### Object Container

```cpp
#include <idacpp/core/core.hpp>

using namespace idacpp::core;

objcontainer_t<my_object_t> objects;

// Create object (automatically managed)
auto* obj = objects.create(constructor_args...);

// Access by index
auto* first = objects[0];
auto* last = objects[-1];

// Automatic cleanup when container goes out of scope
```


## Building Examples

```bash
cmake -B build
cmake --build build
```

Examples are built as IDA plugins demonstrating each module's functionality.

## Project Structure

```
idacpp/
├── include/idacpp/        # Public headers
│   ├── core/              # Core utilities
│   ├── kernwin/           # UI and actions
│   ├── hexrays/           # Decompiler utilities
│   ├── expr/              # Expression utilities
│   ├── callbacks/         # Callback utilities
│   └── idacpp.hpp         # Master include
├── examples/              # Example IDA plugins
├── CMakeLists.txt
├── CLAUDE.md             # Architecture documentation
└── README.md
```

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Author

Elias Bachaalany - [@0xeb](https://github.com/0xeb)

## Contributing

Contributions welcome! This project follows modern C++ best practices and maintains consistency with the IDA SDK's patterns.
