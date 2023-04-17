#include <type_traits>

template<typename Container, typename WrapperIterator>
class OwningRange {
public:
    explicit OwningRange(Container container) noexcept(noexcept(Container(std::move(container))))
            : container_(std::move(container)) {
    }

    [[nodiscard]] auto begin() const noexcept(noexcept(WrapperIterator(container_.begin()))) {
        return WrapperIterator(container_.begin());
    }

    [[nodiscard]] auto end() const noexcept(noexcept(WrapperIterator(container_.end()))) {
        return WrapperIterator(container_.end());
    }

    [[nodiscard]] auto begin() noexcept(noexcept(WrapperIterator(container_.begin()))) {
        return WrapperIterator(container_.begin());
    }

    [[nodiscard]] auto end() noexcept(noexcept(WrapperIterator(container_.end()))) {
        return WrapperIterator(container_.end());
    }

    [[nodiscard]] auto Release() noexcept {
        return std::move(container_);
    }

private:
    Container container_;
};

template<typename Container>
class OwningRange<Container, void> {
public:
    explicit OwningRange(Container container) noexcept(noexcept(Container(std::move(container))))
            : container_(std::move(container)) {
    }

    [[nodiscard]] auto begin() const noexcept(noexcept(container_.begin())) {
        return container_.begin();
    }

    [[nodiscard]] auto end() const noexcept(noexcept(container_.end())) {
        return container_.end();
    }

    [[nodiscard]] auto begin() noexcept(noexcept(container_.begin())) {
        return container_.begin();
    }

    [[nodiscard]] auto end() noexcept(noexcept(container_.end())) {
        return container_.end();
    }

    [[nodiscard]] auto Release() noexcept {
        return std::move(container_);
    }

private:
    Container container_;
};

template<typename WrapperIter = void, typename Container>
auto MakeOwningRange(Container&& container) {
    return OwningRange<std::remove_reference_t<Container>, WrapperIter>(std::forward<Container>(container));
}