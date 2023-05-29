#pragma once

#include <utility>

template<typename TopIterCategoryTag, typename BottomIterCategoryTag>
constexpr auto DeducingFlattenIteratorCategory() {
    using std::is_same_v;

    if constexpr (is_same_v<BottomIterCategoryTag, std::random_access_iterator_tag>) {
        if constexpr (is_same_v<TopIterCategoryTag, std::random_access_iterator_tag>) {
            return std::bidirectional_iterator_tag{};
        } else {
            return TopIterCategoryTag{};
        }
    } else if constexpr (is_same_v<BottomIterCategoryTag, std::bidirectional_iterator_tag>) {
        if constexpr (is_same_v<TopIterCategoryTag, std::forward_iterator_tag>) {
            return std::forward_iterator_tag{};
        } else {
            return std::bidirectional_iterator_tag{};
        }
    } else if constexpr (is_same_v<BottomIterCategoryTag, std::forward_iterator_tag>) {
        return std::forward_iterator_tag{};
    } else {
        static_assert(is_same_v<TopIterCategoryTag, std::input_iterator_tag>
                   || is_same_v<BottomIterCategoryTag, std::input_iterator_tag>,
                   "The iterator category must be the forward iterator category at least.");
        return;
    }
}

/// This flatten container owns a container of containers and
/// behaves as if it contains all elements of the bottom containers in the same order
/// as if these elements were iterated over in two cycles like this:
///
/// \code
/// for (const auto& container : containers) {
///     for (const auto& elem : container) {
///         elem;
///     }
/// }
/// \endcode
///
/// Requirement: all containers have iterators with a category LegacyForwardIterator at least.
/// Restriction: this container is not suitable for algorithms requiring LegacyRandomAccessIterator.
template<typename Container>
class FlattenContainer {
public:
    using value_type = typename Container::value_type::value_type;
    using reference = typename Container::value_type::reference;
    using const_reference = typename Container::value_type::const_reference;
    using difference_type = typename Container::value_type::difference_type;
    using size_type = typename Container::value_type::size_type;

    template<typename TopIter>
    class Iterator {
    private:
        friend FlattenContainer;

        using BottomIter = std::conditional_t<std::is_same_v<TopIter, typename Container::const_iterator>,
                typename TopIter::value_type::const_iterator,
                typename TopIter::value_type::iterator>;

        using TopIterCategory = typename std::iterator_traits<TopIter>::iterator_category;
        using BottomIterCategory = typename std::iterator_traits<BottomIter>::iterator_category;

        void ForwardToFirstNonemptyBottomContainer()
        noexcept(noexcept(std::declval<TopIter>() != std::declval<TopIter>())
              && noexcept(IsBottomEmpty())
              && noexcept(std::is_nothrow_copy_constructible_v<BottomIter>)
              && noexcept(++current_top_iter_)) {
            while (current_top_iter_ != end_top_iter_) {
                if (!IsBottomEmpty()) {
                    current_bottom_iter_ = BottomBegin();
                    break;
                }
                ++current_top_iter_;
            }
        }

        void BackwardToFirstNonemptyBottomContainer()
        noexcept(noexcept(IsBottomEmpty())
              && noexcept(std::is_nothrow_copy_constructible_v<BottomIter>)
              && noexcept(--current_top_iter_)
              && noexcept(--current_bottom_iter_)) {
            do {
                --current_top_iter_;
            } while (IsBottomEmpty());
            current_bottom_iter_ = BottomEnd();
            --current_bottom_iter_;
        }

        explicit Iterator(TopIter current_top_iter, TopIter end_top_iter)
                noexcept(std::is_nothrow_move_constructible_v<TopIter>
                      && noexcept(ForwardToFirstNonemptyBottomContainer()))
                : current_top_iter_(std::move(current_top_iter))
                , end_top_iter_(std::move(end_top_iter)) {
            ForwardToFirstNonemptyBottomContainer();
        }

    public:
        using iterator_category = decltype(DeducingFlattenIteratorCategory<TopIterCategory, BottomIterCategory>());

        using value_type = typename std::iterator_traits<BottomIter>::value_type;
        using difference_type = typename std::iterator_traits<BottomIter>::difference_type;
        using pointer = typename std::iterator_traits<BottomIter>::pointer;
        using reference = typename std::iterator_traits<BottomIter>::reference;

        explicit Iterator() = default;

        reference operator*() noexcept(noexcept(*std::declval<BottomIter>())) {
            return *current_bottom_iter_;
        }

        pointer operator->() noexcept(noexcept(&**this)) {
            return &**this;
        }

        bool operator==(const Iterator& rhs) const
                noexcept(noexcept(std::declval<const TopIter>() == std::declval<const TopIter>())
                      && noexcept(std::declval<const BottomIter>() == std::declval<const BottomIter>())) {
            return current_top_iter_ == rhs.current_top_iter_
               && (current_top_iter_ == end_top_iter_ || current_bottom_iter_ == rhs.current_bottom_iter_);
        }

        bool operator!=(const Iterator& rhs) const noexcept(noexcept(!(*this == rhs))) {
            return !(*this == rhs);
        }

        Iterator& operator++()
                noexcept(noexcept(++std::declval<BottomIter>())
                      && noexcept(std::declval<const BottomIter>() == std::declval<const BottomIter>())
                      && noexcept(ForwardToFirstNonemptyBottomContainer())) {
            ++current_bottom_iter_;
            if (current_bottom_iter_ == BottomEnd()) {
                ++current_top_iter_;
                ForwardToFirstNonemptyBottomContainer();
            }
            return *this;
        }

        Iterator operator++(int)
                noexcept(std::is_nothrow_copy_constructible_v<Iterator> && noexcept(++(*this))) {
            auto this_copy = *this;
            (void) ++(*this);
            return this_copy;
        }

        Iterator& operator--()
                noexcept(noexcept(--std::declval<BottomIter>())
                      && noexcept(std::declval<const BottomIter>() == std::declval<const BottomIter>())
                      && noexcept(BackwardToFirstNonemptyBottomContainer())) {
            if (current_top_iter_ == end_top_iter_ || current_bottom_iter_ == BottomBegin()) {
                BackwardToFirstNonemptyBottomContainer();
            } else {
                --current_bottom_iter_;
            }
            return *this;
        }

        Iterator operator--(int)
                noexcept(std::is_nothrow_copy_constructible_v<Iterator> && noexcept(--(*this))) {
            auto this_copy = *this;
            (void) --(*this);
            return this_copy;
        }

        void swap(Iterator& rhs)
                noexcept(std::is_nothrow_swappable_v<TopIter> && std::is_nothrow_swappable_v<BottomIter>) {
            using std::swap;
            swap(current_top_iter_, rhs.current_top_iter_);
            swap(end_top_iter_, rhs.end_top_iter_);
            swap(current_bottom_iter_, rhs.current_bottom_iter_);
        }

        friend void swap(Iterator& lhs, Iterator& rhs) noexcept(noexcept(lhs.swap(rhs))) {
            lhs.swap(rhs);
        }

    private:
        TopIter current_top_iter_;
        TopIter end_top_iter_;
        BottomIter current_bottom_iter_;

        BottomIter BottomBegin() const noexcept(noexcept(current_top_iter_->begin())) {
            return current_top_iter_->begin();
        }

        BottomIter BottomEnd() const noexcept(noexcept(current_top_iter_->end())) {
            return current_top_iter_->end();
        }

        bool IsBottomEmpty() const noexcept(noexcept(BottomBegin() == BottomEnd())) {
            return BottomBegin() == BottomEnd();
        }
    };

    using iterator = Iterator<typename Container::iterator>;
    using const_iterator = Iterator<typename Container::const_iterator>;

    FlattenContainer() noexcept(std::is_nothrow_constructible_v<Container>) = default;

    explicit FlattenContainer(Container container)
            : container_(std::move(container)) {
    }

    [[nodiscard]] iterator begin()
            noexcept(std::is_nothrow_constructible_v<
                     iterator,
                     typename Container::iterator, typename Container::iterator>) {
        return iterator(container_.begin(), container_.end());
    }

    [[nodiscard]] iterator end()
            noexcept(std::is_nothrow_constructible_v<
                     iterator,
                     typename Container::iterator, typename Container::iterator>) {
        return iterator(container_.end(), container_.end());
    }

    [[nodiscard]] const_iterator begin() const noexcept(noexcept(cbegin())) {
        return cbegin();
    }

    [[nodiscard]] const_iterator end() const noexcept(noexcept(cend())) {
        return cend();
    }

    [[nodiscard]] const_iterator cbegin() const
            noexcept(std::is_nothrow_constructible_v<
                     const_iterator,
                     typename Container::const_iterator, typename Container::const_iterator>) {
        return const_iterator(container_.cbegin(), container_.cend());
    }

    [[nodiscard]] const_iterator cend() const
            noexcept(std::is_nothrow_constructible_v<
                     const_iterator,
                     typename Container::const_iterator, typename Container::const_iterator>) {
        return const_iterator(container_.cend(), container_.cend());
    }

    [[nodiscard]] Container&& Release() noexcept {
        return std::move(container_);
    }

private:
    Container container_;
};

template<typename Container>
auto MakeFlattenContainer(Container&& container) {
    return FlattenContainer<std::remove_reference_t<Container>>(std::forward<Container>(container));
}