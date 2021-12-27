/*
IDASDK extension library (c) Elias Bachaalany.

Base utilities
*/
#pragma once

#include <functional>
#include <vector>
#include <memory>

//----------------------------------------------------------------------------------
// An object container. It owns the objects and when it dies, everything dies with it.
template <typename T>
class objcontainer_t: public std::vector<std::unique_ptr<T>>
{
    using base_t = std::vector<std::unique_ptr<T>>;
public:
    template<typename... Args>
    T* create(Args&&... args)
    {
        push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return back().get();
    }

    T* operator[](int index)
    {
        size_t idx;
        if (index < 0)
            idx = size() - size_t(-index);
        else
            idx = size_t(index);

        if (idx >= size())
            return nullptr;
        else
            return base_t::operator [](idx).get();
    }
};
