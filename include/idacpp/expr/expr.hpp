/*
idacpp - Modern C++ extensions for IDA SDK
Copyright (c) 2025 Elias Bachaalany <elias.bachaalany@gmail.com>

Expression utilities module
*/
#pragma once

#include <expr.hpp>

namespace idacpp::expr
{

//-------------------------------------------------------------------------
/**
 * @brief Find and cache the Python external language object.
 *
 * Searches for the Python external language plugin and caches the result
 * for subsequent calls.
 *
 * @param force If true, force re-search even if cached
 * @return Pointer to Python extlang_t, or nullptr if not found
 *
 * @note This function is useful when you need to evaluate Python expressions
 *       or interact with IDA's Python integration from C++.
 */
inline extlang_t* pylang(bool force = false)
{
    struct find_python : extlang_visitor_t
    {
        extlang_t** pylang;

        virtual ssize_t idaapi visit_extlang(extlang_t* extlang) override
        {
            if (streq(extlang->fileext, "py"))
            {
                *pylang = extlang;
                return 1;
            }
            return 0;
        }

        find_python(extlang_t** pylang) : pylang(pylang)
        {
            for_all_extlangs(*this, false);
        }
    };

    static extlang_t* s_pylang = nullptr;
    if (force || s_pylang == nullptr)
    {
        s_pylang = nullptr;
        find_python{&s_pylang};
    }

    return s_pylang;
}

}  // namespace idacpp::expr
