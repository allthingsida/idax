/*
idacpp - Modern C++ extensions for IDA SDK
Copyright (c) 2025 Elias Bachaalany <elias.bachaalany@gmail.com>

Master convenience header - includes all idacpp modules
*/
#pragma once

// Core utilities
#include <idacpp/core/core.hpp>

// Kernwin utilities (UI and actions)
#include <idacpp/kernwin/kernwin.hpp>

// Hexrays utilities (decompiler)
#include <idacpp/hexrays/hexrays.hpp>

// Expression utilities
#include <idacpp/expr/expr.hpp>

// Callback utilities
#include <idacpp/callbacks/callbacks.hpp>

/**
 * @mainpage idacpp - Modern C++ Extensions for IDA SDK
 *
 * @section intro_sec Introduction
 *
 * idacpp is a modern C++20 library providing high-level utilities and abstractions
 * for IDA Pro plugin development. It is the successor to idax, redesigned with
 * proper namespacing, modern C++ features, and ida-cmake integration.
 *
 * @section modules_sec Modules
 *
 * - @ref idacpp::core "Core" - Base utilities and containers
 * - @ref idacpp::kernwin "Kernwin" - UI and action management
 * - @ref idacpp::hexrays "Hexrays" - Decompiler utilities
 * - @ref idacpp::expr "Expr" - Expression evaluation
 * - @ref idacpp::callbacks "Callbacks" - C API bridging
 *
 * @section usage_sec Quick Start
 *
 * @code
 * #include <idacpp/idacpp.hpp>  // All modules
 *
 * using namespace idacpp::kernwin;
 *
 * action_manager_t actions;
 * actions.add_action(...);
 * @endcode
 *
 * @section license_sec License
 *
 * MIT License - see LICENSE file for details
 */
