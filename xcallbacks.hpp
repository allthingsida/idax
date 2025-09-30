/*
IDASDK extension library (c) Elias Bachaalany.

Callback utilities
*/
#pragma once

#include <functional>
#include <vector>
#include <shared_mutex>
#include <optional>
#include <type_traits>
#include <utility>
#include <cstdint>
#include <array>
#include <mutex>

//------------------------------------------------------------------------------
// Opaque handle for registered callbacks
using callback_handle_t = uint32_t;
constexpr callback_handle_t INVALID_CALLBACK_HANDLE = 0;

//------------------------------------------------------------------------------
// Single global callback registry for bridging C APIs with C++ lambdas
// Uses handle-based API for lifecycle management
// Tag parameter allows multiple independent registries with same signature
template <typename CPrototype, size_t MaxCallbacks = 256, typename Tag = void>
class callback_registry
{
public:
    using callback_t = CPrototype;
    using lambda_t = std::function<std::remove_pointer_t<CPrototype>>;

    // Singleton access
    static callback_registry& instance()
    {
        static callback_registry inst;
        return inst;
    }

private:
    callback_registry() = default;
    ~callback_registry() = default;

    callback_registry(const callback_registry&) = delete;
    callback_registry& operator=(const callback_registry&) = delete;

public:
    // Register a callback, returns handle and C function pointer
    std::optional<std::pair<callback_handle_t, callback_t>> register_callback(lambda_t cb)
    {
        std::unique_lock lock(mutex_);

        // Find available slot
        for (size_t i = 0; i < MaxCallbacks; ++i)
        {
            if (!callbacks_[i])
            {
                callback_handle_t handle = next_handle_++;
                callbacks_[i] = std::move(cb);
                handles_[i] = handle;
                return std::make_pair(handle, get_wrapper_for_index(i));
            }
        }

        return std::nullopt; // Registry full
    }

    // Unregister by handle
    bool unregister_callback(callback_handle_t handle)
    {
        if (handle == INVALID_CALLBACK_HANDLE)
            return false;

        std::unique_lock lock(mutex_);

        for (size_t i = 0; i < MaxCallbacks; ++i)
        {
            if (handles_[i] == handle && callbacks_[i])
            {
                callbacks_[i] = nullptr;
                handles_[i] = INVALID_CALLBACK_HANDLE;
                return true;
            }
        }

        return false;
    }

    // Unregister all callbacks (clear the registry)
    void unregister_all()
    {
        std::unique_lock lock(mutex_);

        for (size_t i = 0; i < MaxCallbacks; ++i)
        {
            callbacks_[i] = nullptr;
            handles_[i] = INVALID_CALLBACK_HANDLE;
        }
    }

    // Diagnostics
    size_t size() const
    {
        std::shared_lock lock(mutex_);
        size_t count = 0;
        for (const auto& cb : callbacks_)
            if (cb)
                ++count;
        return count;
    }

    size_t capacity() const { return MaxCallbacks; }

private:
    // Static wrapper function for index
    template <size_t Index>
    static auto wrapper_function(auto... args)
        -> decltype(std::declval<lambda_t>()(args...))
    {
        using return_t = decltype(std::declval<lambda_t>()(args...));

        auto& self = instance();

        // Copy callback to avoid concurrent unregistration issues
        lambda_t callback_copy;
        {
            std::shared_lock lock(self.mutex_);
            if (Index < MaxCallbacks)
                callback_copy = self.callbacks_[Index];
        }

        // Call outside lock
        if (callback_copy)
            return callback_copy(args...);

        if constexpr (!std::is_void_v<return_t>)
            return return_t{};
    }

    // Generate array of wrapper functions at compile time
    template <size_t... Is>
    static constexpr auto make_wrapper_array(std::index_sequence<Is...>)
    {
        return std::array<callback_t, sizeof...(Is)>{ &wrapper_function<Is>... };
    }

    // Get wrapper for specific index
    static callback_t get_wrapper_for_index(size_t index)
    {
        static constexpr auto wrappers = make_wrapper_array(
            std::make_index_sequence<MaxCallbacks>{}
        );

        if (index < MaxCallbacks)
            return wrappers[index];

        return nullptr;
    }

    std::array<lambda_t, MaxCallbacks> callbacks_{};
    std::array<callback_handle_t, MaxCallbacks> handles_{};
    callback_handle_t next_handle_ = 1;
    mutable std::shared_mutex mutex_;
};

//------------------------------------------------------------------------------
// Macro to define a callback registry with tag-based disambiguation
#define DEFINE_CALLBACK_REGISTRY(name, Prototype, MaxCallbacks) \
    struct name##_tag {};                                       \
    using name##_type = callback_registry<Prototype, MaxCallbacks, name##_tag>; \
    inline name##_type& name = name##_type::instance();

//------------------------------------------------------------------------------
// Simplified API functions

template <typename CPrototype, size_t MaxCallbacks = 256, typename Tag = void, typename Lambda>
inline std::optional<std::pair<callback_handle_t, CPrototype>>
register_callback(Lambda&& cb)
{
    return callback_registry<CPrototype, MaxCallbacks, Tag>::instance()
        .register_callback(std::forward<Lambda>(cb));
}

template <typename CPrototype, size_t MaxCallbacks = 256, typename Tag = void>
inline bool unregister_callback(callback_handle_t handle)
{
    return callback_registry<CPrototype, MaxCallbacks, Tag>::instance()
        .unregister_callback(handle);
}

//------------------------------------------------------------------------------
// RAII wrapper for automatic unregistration
template <typename CPrototype, size_t MaxCallbacks = 256, typename Tag = void>
class scoped_callback
{
public:
    using callback_t = CPrototype;
    using registry_t = callback_registry<CPrototype, MaxCallbacks, Tag>;

    template <typename Lambda>
    explicit scoped_callback(Lambda&& cb)
        : handle_(INVALID_CALLBACK_HANDLE), callback_(nullptr)
    {
        auto result = registry_t::instance().register_callback(std::forward<Lambda>(cb));
        if (result)
        {
            handle_ = result->first;
            callback_ = result->second;
        }
    }

    ~scoped_callback()
    {
        reset();
    }

    // Move semantics
    scoped_callback(scoped_callback&& other) noexcept
        : handle_(std::exchange(other.handle_, INVALID_CALLBACK_HANDLE)),
          callback_(std::exchange(other.callback_, nullptr)) {}

    scoped_callback& operator=(scoped_callback&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            handle_ = std::exchange(other.handle_, INVALID_CALLBACK_HANDLE);
            callback_ = std::exchange(other.callback_, nullptr);
        }
        return *this;
    }

    // Non-copyable
    scoped_callback(const scoped_callback&) = delete;
    scoped_callback& operator=(const scoped_callback&) = delete;

    // Access
    callback_t get() const { return callback_; }
    callback_t operator*() const { return callback_; }

    explicit operator bool() const { return callback_ != nullptr; }
    bool is_valid() const { return callback_ != nullptr; }

    // Manual control
    void reset()
    {
        if (handle_ != INVALID_CALLBACK_HANDLE)
        {
            registry_t::instance().unregister_callback(handle_);
            handle_ = INVALID_CALLBACK_HANDLE;
            callback_ = nullptr;
        }
    }

    callback_t release()
    {
        handle_ = INVALID_CALLBACK_HANDLE;
        return std::exchange(callback_, nullptr);
    }

    callback_handle_t handle() const { return handle_; }

private:
    callback_handle_t handle_;
    callback_t callback_;
};

//------------------------------------------------------------------------------
// Helper factory function
template <typename CPrototype, size_t MaxCallbacks = 256, typename Tag = void, typename Lambda>
auto make_scoped_callback(Lambda&& cb)
{
    return scoped_callback<CPrototype, MaxCallbacks, Tag>(std::forward<Lambda>(cb));
}
