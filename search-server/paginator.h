#pragma once

#include <cstddef>
#include <iterator>
#include <ostream>
#include <type_traits>

template<typename T>
class Range {
public:
    Range(T begin, T end);

    T begin() const noexcept;

    T end() const noexcept;

private:
    T begin_;
    T end_;
};

template<typename InputIterator>
class Page {
public:
    Page(InputIterator begin, InputIterator end_of_pages, std::size_t page_size) noexcept;

    InputIterator begin() const noexcept;

    InputIterator end() const noexcept;

    bool operator!=(Page rhs) const noexcept;

    Page& operator++() noexcept;

    Range<InputIterator> operator*() const noexcept;

private:
    Range<InputIterator> range_;
    const InputIterator end_of_pages_;
    const std::size_t size_;

    static InputIterator GetEnd(InputIterator begin, InputIterator end_of_pages, std::size_t page_size) noexcept;
};

template<typename InputIterator>
class Paginator {
public:
    Paginator(InputIterator range_begin, InputIterator range_end, std::size_t page_size) noexcept;

    Page<InputIterator> begin() const noexcept;

    Page<InputIterator> end() const noexcept;

private:
    InputIterator range_begin_;
    InputIterator range_end_;
    std::size_t page_size_;
};

// Range template implementation

template<typename T>
Range<T>::Range(T begin, T end)
        : begin_(begin)
        , end_(end) {
}

template<typename T>
T Range<T>::begin() const noexcept {
    return begin_;
}

template<typename T>
T Range<T>::end() const noexcept {
    return end_;
}

// The end of Range template implementation

// Page template implementation

template<typename InputIterator>
Page<InputIterator>::Page(InputIterator begin, InputIterator end_of_pages, std::size_t page_size) noexcept
        : range_(begin, GetEnd(begin, end_of_pages, page_size))
        , end_of_pages_(end_of_pages)
        , size_(page_size) {
}

template<typename InputIterator>
InputIterator Page<InputIterator>::begin() const noexcept {
    return range_.begin();
}

template<typename InputIterator>
InputIterator Page<InputIterator>::end() const noexcept {
    return range_.end();
}

template<typename InputIterator>
bool Page<InputIterator>::operator!=(Page rhs) const noexcept {
    return range_.begin() != rhs.range_.begin() || size_ != rhs.size_;
}

template<typename InputIterator>
Page<InputIterator>& Page<InputIterator>::operator++() noexcept {
    range_ = Range(end(), GetEnd(range_.end(), end_of_pages_, size_));
    return *this;
}

template<typename InputIterator>
Range<InputIterator> Page<InputIterator>::operator*() const noexcept {
    return range_;
}

template<typename InputIterator>
InputIterator Page<InputIterator>::GetEnd(InputIterator begin, InputIterator end_of_pages,
                                          std::size_t page_size) noexcept {
    if (std::is_same_v<typename InputIterator::iterator_category, std::random_access_iterator_tag>) {
        if (page_size >= static_cast<std::size_t>(std::abs(end_of_pages - begin))) {
            return end_of_pages;
        }
        return begin + page_size;
    } else {
        auto it = begin;
        for (std::size_t count = 0; count != page_size; ++count, ++it) {
            if (it == end_of_pages) {
                break;
            }
        }
        return it;
    }
}

// The end of Page template implementation

template<typename T>
std::ostream& operator<<(std::ostream& os, Range<T> range) {
    for (auto it = range.begin(); it != range.end(); ++it) {
        os << *it;
    }
    return os;
}

// Paginator template implementation

template<typename InputIterator>
Paginator<InputIterator>::Paginator(InputIterator range_begin, InputIterator range_end, std::size_t page_size) noexcept
        : range_begin_(range_begin)
        , range_end_(page_size != 0 ? range_end : range_begin)
        , page_size_(page_size) {
}

template<typename InputIterator>
Page<InputIterator> Paginator<InputIterator>::begin() const noexcept {
    return Page(range_begin_, range_end_, page_size_);
}

template<typename InputIterator>
Page<InputIterator> Paginator<InputIterator>::end() const noexcept {
    return Page(range_end_, range_end_, page_size_);
}

// The end of Paginator template implementation

template <typename Container>
auto Paginate(const Container& c, std::size_t page_size) noexcept(noexcept(std::begin(c)) && noexcept(std::end(c))) {
    return Paginator(std::begin(c), std::end(c), page_size);
}