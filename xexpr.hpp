/*
IDASDK extension library (c) Elias Bachaalany.

Expressions utilities
*/
#pragma once

#include <expr.hpp>

//-------------------------------------------------------------------------
// Finds the Python external language object (once and caches it)
inline extlang_t* pylang(bool force=false)
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
        find_python{ &s_pylang };
    }

    return s_pylang;
}
