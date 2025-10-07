# idacpp Examples

This directory contains example IDA plugins demonstrating the usage of various idacpp modules.

## Building Examples

The examples are built using ida-cmake. Make sure you have:
- IDA SDK with ida-cmake installed
- `IDASDK` environment variable set
- CMake 3.27 or later

```bash
# Configure
cmake -B build

# Build (RelWithDebInfo is recommended for IDA plugins)
cmake --build build --config RelWithDebInfo

# Or build Debug for development
cmake --build build --config Debug
```

The compiled plugins will be in `build/<config>/` directory.

## Running Examples

1. Copy the compiled `.dll` (Windows) or `.so` (Linux) files to your IDA plugins directory
2. Restart IDA or use `File > Load > Load File` to load the plugin
3. Check the Output window for messages from the examples

## Examples Overview

### Core Module

**`objcontainer_example`** - Demonstrates `idacpp::core::objcontainer_t`
- RAII container for automatic object lifetime management
- Positive and negative indexing
- Shows automatic cleanup on scope exit

### Kernwin Module

**`action_manager_example`** - Demonstrates `idacpp::kernwin::action_manager_t`
- Creating actions with lambda handlers
- Using default enable state handlers
- Attaching actions to menus and popups
- Icon usage with `IDAICONS`
- Hotkey binding

Key Features:
- `Ctrl-Shift-H`: Print hello message
- Right-click in views: Context menu actions
- Widget-specific enable states

### Hexrays Module

**`visitor_example`** - Demonstrates `idacpp::hexrays` utilities
- `hexrays_ctreeparent_visitor_t` for parent tracking
- Walking the parent chain of selected items
- Finding items by effective address
- `hexrays_find_expr()` for expression searching

Usage:
- Open a decompiler view
- Right-click for context menu actions
- Select an expression to see its parent chain

## Module-Specific Notes

### Core
- Header-only, no IDA-specific dependencies
- Can be used in any C++20 project

### Kernwin
- Requires IDA GUI mode
- Actions are automatically cleaned up when plugin unloads

### Hexrays
- Requires Hexrays Decompiler
- Examples check for decompiler availability at init
- Uses `init_hexrays_plugin()` / `term_hexrays_plugin()`

### Expr
- Expression evaluation utilities
- Python interop helpers

### Callbacks
- Bridge C APIs with C++ lambdas
- Thread-safe callback registry
- RAII wrapper for automatic cleanup

## Creating Your Own Plugin

Use these examples as templates:

```cpp
#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>

#include <idacpp/idacpp.hpp>  // All modules

using namespace idacpp::kernwin;

struct plugin_ctx_t : public plugmod_t
{
    action_manager_t actions;

    plugin_ctx_t() : actions(this)
    {
        // Your initialization
    }

    virtual ~plugin_ctx_t()
    {
        // Cleanup
        actions.remove_actions();
    }

    virtual bool idaapi run(size_t arg) override
    {
        // Plugin logic
        return true;
    }
};

static plugmod_t* idaapi init()
{
    return new plugin_ctx_t;
}

plugin_t PLUGIN =
{
    IDP_INTERFACE_VERSION,
    PLUGIN_MULTI,
    init,
    nullptr,
    nullptr,
    "My Plugin",
    "Description",
    "Short Name",
    nullptr
};
```

## CMakeLists.txt Template

```cmake
ida_add_plugin(my_plugin
    SOURCES
        my_plugin.cpp
    OUTPUT_NAME
        my_plugin
)

target_link_libraries(my_plugin PRIVATE idacpp::idacpp)
```

## Troubleshooting

**Build fails with "idasdk not found"**
- Ensure `IDASDK` environment variable is set
- Check that `$IDASDK/ida-cmake/bootstrap.cmake` exists

**Plugin doesn't load**
- Check IDA version compatibility
- Verify plugin is in correct architecture (32-bit vs 64-bit)
- Check IDA Output window for error messages

**Hexrays examples don't work**
- Ensure you have Hexrays Decompiler installed
- Open a decompiler view before using the actions

## Learning Path

Recommended order for learning idacpp:

1. **core** - Understand basic utilities
2. **kernwin** - Learn action management
3. **hexrays** - Explore decompiler integration
4. **expr** - Expression evaluation
5. **callbacks** - Advanced C API bridging

## Additional Resources

- [idacpp Documentation](../README.md)
- [IDA SDK Documentation](https://hex-rays.com/products/ida/support/sdkdoc/)
- [ida-cmake](https://github.com/0xeb/ida-cmake)
