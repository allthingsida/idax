# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

**idacpp** is a modern C++20 header-only library providing high-level utilities and abstractions for IDA Pro plugin development with proper namespacing (`idacpp::module`), modern C++ features, and ida-cmake integration.

## Build System

### CMake Integration

This is a **header-only library** with ida-cmake integration:

```bash
# Build examples (requires IDA SDK with ida-cmake)
cmake -B build
cmake --build build
```

- **CMake version**: 3.27+
- **C++ standard**: C++20 (required)
- **IDA SDK integration**: Uses `ida-cmake` bootstrap from `$IDASDK/ida-cmake/bootstrap.cmake`
- **CMake target**: `idacpp::idacpp` (INTERFACE library)

### Project Structure

```
include/idacpp/          # All public headers (header-only)
├── core/                # RAII containers and utilities
├── kernwin/             # UI, actions, icon constants
├── hexrays/             # Decompiler ctree visitors and utilities
├── expr/                # Expression evaluation (Python integration)
├── callbacks/           # C API to C++ lambda bridging
└── idacpp.hpp           # Master include (all modules)

examples/                # Example IDA plugins demonstrating each module
├── core/                # objcontainer_t examples
├── kernwin/             # action_manager_t examples
└── hexrays/             # ctree visitor examples
```

## Architecture

### Namespace Organization

All code is organized under `idacpp::module` namespaces:

- **`idacpp::core`** - Base utilities (objcontainer_t)
- **`idacpp::kernwin`** - UI and action management
- **`idacpp::hexrays`** - Decompiler utilities
- **`idacpp::expr`** - Expression evaluation
- **`idacpp::callbacks`** - Callback registry system

**Critical**: When editing files, preserve the namespace structure. Classes and functions belong to their respective module namespaces.

### Module Dependencies

- **core** - No dependencies (self-contained)
- **kernwin** - Depends on `core` (uses `objcontainer_t`)
- **hexrays** - Depends on `kernwin` (uses action helpers)
- **expr** - Independent
- **callbacks** - Independent

### Key Design Patterns

#### 1. RAII Object Management (core)

`objcontainer_t<T>` provides automatic lifetime management:
- Stores objects with `unique_ptr`
- Negative indexing support (`[-1]` = last element)
- Example: `action_manager_t` stores action handlers in `objcontainer_t`

#### 2. Function Object Action Handlers (kernwin)

Action system bridges IDA's C API to C++ lambdas:
- `fo_action_handler_ah_t` - Wraps lambdas as action handlers
- `FO_ACTION_UPDATE` macro - Creates update/state callbacks
- `FO_ACTION_ACTIVATE` macro - Creates activation callbacks
- `action_manager_t` - Manages action lifecycle and popup attachment

#### 3. Ctree Parent Tracking (hexrays)

`hexrays_ctreeparent_visitor_t` extends `ctree_parentee_t`:
- Builds parent relationship map during traversal
- Maps effective addresses to ctree items
- Provides ancestry checking (`is_ancestor_of`)
- Used for statement-level transformations

#### 4. Callback Registry (callbacks)

Thread-safe C API to C++ lambda bridging:
- Compile-time wrapper generation (no runtime overhead)
- Handle-based lifecycle management
- `scoped_callback<T>` for RAII
- Supports tag-based disambiguation for multiple registries


## Common Development Tasks

### Adding New Utilities to Existing Modules

When adding new utilities to a module (e.g., `kernwin`):
1. Add to `include/idacpp/module/module.hpp`
2. Keep code in `namespace idacpp::module { ... }`
3. Document with Doxygen comments
4. Optionally add example in `examples/module/`

### Creating Examples

Examples are IDA plugins built with ida-cmake:
1. Add subdirectory in `examples/module/`
2. Create `CMakeLists.txt` with `ida_add_plugin()`
3. Link to `idacpp::idacpp`
4. Include relevant module headers

### Icon Constants (kernwin)

`IDAICONS` namespace provides named constants for IDA's built-in icons:
- `GREEN_DOT`, `RED_DOT` - Breakpoint indicators
- `GREEN_PLAY_BUTTON` - Start process
- `YELLOW_COG_WHEEL` - Settings
- `LIGHT_BULB` - Hints
- And more...

Use these instead of magic numbers when creating actions.

## Style and Conventions

- **C++ Standard**: C++20 features encouraged (concepts, ranges, etc.)
- **Templates**: Prefer template parameters over macros where possible
- **Macros**: Only for code generation (e.g., `FO_ACTION_UPDATE`)
- **Documentation**: Doxygen comments for all public APIs
- **Naming**: Snake_case with `_t` suffix for types
- **RAII**: Prefer RAII wrappers over manual cleanup

## Testing

Examples serve as integration tests. Each example demonstrates:
- Correct API usage
- Integration with IDA SDK
- Real-world plugin patterns

To verify changes, build and load examples in IDA Pro.
