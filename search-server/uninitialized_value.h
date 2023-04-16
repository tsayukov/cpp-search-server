#pragma once

#include <type_traits>

template<typename T, std::size_t N = sizeof(T)>
class UninitializedValue {
private:
    void Drop() noexcept(std::is_nothrow_destructible_v<T>) {
        if constexpr (std::is_destructible_v<T>) {
            if (is_initialized) {
                Get().~T();
            }
        }
    }

public:
    UninitializedValue() noexcept = default;

    ~UninitializedValue() noexcept {
        Drop();
    }

    [[nodiscard]] bool IsInitialized() const noexcept {
        return is_initialized;
    }

    [[nodiscard]] T& Get() noexcept {
        return const_cast<T&>(const_cast<const UninitializedValue&>(*this).Get());
    }

    [[nodiscard]] const T& Get() const noexcept {
        return *reinterpret_cast<const T*>(raw_placement_);
    }

    template<typename ...Args>
    void Set(Args&&... args) noexcept(noexcept(Drop()) && noexcept(T(std::forward<Args...>(args)...))) {
        Drop();
        T* value_ptr = reinterpret_cast<T*>(raw_placement_);
        new (value_ptr) T(std::forward<Args...>(args)...);
        is_initialized = true;
    }

    [[nodiscard]] T&& Release() noexcept {
        is_initialized = false;
        return std::move(Get());
    }

private:
    std::int8_t raw_placement_[N];
    bool is_initialized = false;
};