#pragma once

#include <type_traits>
#include <utility>

/// This is a container that allows to initialize an element of this container later.
template<typename T>
class LateInitValue {
public:
    LateInitValue() noexcept {
        SetInitializationFlag(false);
    }

    LateInitValue(const LateInitValue& other) noexcept(noexcept(Set(other.Get()))) {
        if (other.IsInitialized()) {
            Set(other.Get());
        }
    }

    LateInitValue& operator=(const LateInitValue& rhs) noexcept(noexcept(Set(rhs.Get()))) {
        if (this != &rhs && rhs.IsInitialized()) {
            Set(rhs.Get());
        }
        return *this;
    }

    LateInitValue(LateInitValue&& other) noexcept(noexcept(Set(other.Release()))) {
        if (other.IsInitialized()) {
            Set(other.Release());
        }
    }

    LateInitValue& operator=(LateInitValue&& rhs) noexcept(noexcept(Set(rhs.Release()))) {
        if (this != &rhs && rhs.IsInitialized()) {
            Set(rhs.Release());
        }
        return *this;
    }

    ~LateInitValue() {
        Drop();
    }

    /// Checks if the element is initialized.
    [[nodiscard]] bool IsInitialized() const noexcept {
        return static_cast<bool>(is_initialized_[0]);
    }

    /// Accessing the element by a reference.
    /// It is undefined behavior if the element is not initialized.
    [[nodiscard]] T& Get() noexcept {
        return const_cast<T&>(const_cast<const LateInitValue&>(*this).Get());
    }

    /// Accessing the element by a const reference.
    /// It is undefined behavior if the element is not initialized.
    [[nodiscard]] const T& Get() const noexcept {
        return *reinterpret_cast<const T*>(raw_placement_);
    }

    /// Constructs a new element by forwarding arguments to the constructor of the element.
    /// If an exception is thrown, this method has no effect (strong exception guarantee).
    template<typename ...Args>
    void Set(Args&&... args) noexcept(noexcept(T(std::forward<Args>(args)...))) {
        if constexpr (noexcept(T(std::forward<Args>(args)...))) {
            NoexceptSet(std::forward<Args>(args)...);
        } else {
            MaybeExceptSet(std::forward<Args>(args)...);
        }
    }

    /// Releases the element.
    /// It is undefined behavior if the element is not initialized.
    [[nodiscard]] T&& Release() noexcept {
        SetInitializationFlag(false);
        return std::move(Get());
    }

private:
    inline static constexpr std::size_t N = sizeof(T);
    inline static constexpr std::size_t Align = alignof(T);

    std::uint8_t raw_placement_[N];
    // Note: an additional alignment is needed because alignof(std::uint8_t[N]) == 1.
    // The first byte is a boolean flag that says whether the element is initialized. Other bytes are not specified.
    std::uint8_t is_initialized_[Align];

    void SetInitializationFlag(bool value) noexcept {
        is_initialized_[0] = value;
    }

    void Drop() noexcept {
        if constexpr (std::is_destructible_v<T>) {
            if (IsInitialized()) {
                Get().~T();
            }
        }
        SetInitializationFlag(false);
    }

    template<typename ...Args>
    void NoexceptSet(Args&&... args) noexcept {
        Drop();
        T* value_ptr = reinterpret_cast<T*>(raw_placement_);
        new (value_ptr) T(std::forward<Args>(args)...);
        SetInitializationFlag(true);
    }

    template<typename ...Args>
    void MaybeExceptSet(Args&&... args) {
        LateInitValue old_state_snapshot;
        if (IsInitialized()) {
            std::memcpy(old_state_snapshot.raw_placement_, raw_placement_, N);
            old_state_snapshot.SetInitializationFlag(true);
        }

        T* value_ptr = reinterpret_cast<T*>(raw_placement_);
        try {
            new (value_ptr) T(std::forward<Args>(args)...);
            SetInitializationFlag(true);
        } catch (...) {
            if (IsInitialized()) {
                std::memcpy(raw_placement_, old_state_snapshot.raw_placement_, N);
                old_state_snapshot.SetInitializationFlag(false);
            }
            throw;
        }
    }
};