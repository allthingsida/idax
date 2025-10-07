/*
idacpp - Modern C++ extensions for IDA SDK
Copyright (c) 2025 Elias Bachaalany <elias.bachaalany@gmail.com>

Core utilities module
*/
#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace idacpp::core
{

//----------------------------------------------------------------------------------
/**
 * @brief RAII object container with automatic lifetime management.
 *
 * An object container that owns its objects. When the container is destroyed,
 * all contained objects are automatically destroyed.
 *
 * @tparam T The type of objects to store
 *
 * @example
 * @code
 * objcontainer_t<my_class_t> objects;
 * auto* obj = objects.create(arg1, arg2);  // Create and store
 * auto* first = objects[0];                // Access by index
 * auto* last = objects[-1];                // Negative indexing
 * // All objects automatically deleted when container goes out of scope
 * @endcode
 */
template <typename T>
class objcontainer_t : public std::vector<std::unique_ptr<T>>
{
    using base_t = std::vector<std::unique_ptr<T>>;

public:
    /**
     * @brief Create and store a new object in the container.
     *
     * @tparam Args Constructor argument types (deduced)
     * @param args Arguments forwarded to T's constructor
     * @return T* Pointer to the newly created object
     */
    template<typename... Args>
    T* create(Args&&... args)
    {
        this->push_back(std::make_unique<T>(std::forward<Args>(args)...));
        return this->back().get();
    }

    /**
     * @brief Access object by index with support for negative indexing.
     *
     * @param index Positive index from start, or negative index from end
     * @return T* Pointer to object at index, or nullptr if out of bounds
     *
     * @note Negative indices count from the end: -1 is last element
     */
    T* operator[](int index)
    {
        size_t idx;
        if (index < 0)
            idx = this->size() - size_t(-index);
        else
            idx = size_t(index);

        if (idx >= this->size())
            return nullptr;
        else
            return base_t::operator[](idx).get();
    }
};

}  // namespace idacpp::core
