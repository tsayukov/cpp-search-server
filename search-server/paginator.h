#pragma once

#include <cstddef>
#include <iterator>
#include <ostream>
#include <type_traits>

template<typename InputIterator>
class Page {
public:
    Page(InputIterator begin, InputIterator end_of_pages, std::size_t page_size) noexcept;

    InputIterator begin() const noexcept;

    InputIterator end() const noexcept;

    bool operator!=(Page rhs) const noexcept;

    Page& operator++() noexcept;

    Page& operator*() noexcept;

private:
    InputIterator begin_;
    InputIterator end_;
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

// Page template implementation

template<typename InputIterator>
Page<InputIterator>::Page(InputIterator begin, InputIterator end_of_pages, std::size_t page_size) noexcept
        : begin_(begin)
        , end_(GetEnd(begin, end_of_pages, page_size))
        , end_of_pages_(end_of_pages)
        , size_(page_size) {
}

template<typename InputIterator>
InputIterator Page<InputIterator>::begin() const noexcept {
    return begin_;
}

template<typename InputIterator>
InputIterator Page<InputIterator>::end() const noexcept {
    return end_;
}

template<typename InputIterator>
bool Page<InputIterator>::operator!=(Page rhs) const noexcept {
    return begin_ != rhs.begin_ || size_ != rhs.size_;
}

template<typename InputIterator>
Page<InputIterator>& Page<InputIterator>::operator++() noexcept {
    begin_ = end();
    end_ = GetEnd(begin_, end_of_pages_, size_);
    return *this;
}

template<typename InputIterator>
Page<InputIterator>& Page<InputIterator>::operator*() noexcept {
    return *this;
}

template<typename InputIterator>
InputIterator Page<InputIterator>::GetEnd(InputIterator begin, InputIterator end_of_pages,
                                          std::size_t page_size) noexcept {
    if (std::is_same_v<typename InputIterator::iterator_category, std::random_access_iterator_tag>) {
        if (page_size >= end_of_pages - begin) {
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

template<typename InputIterator>
std::ostream& operator<<(std::ostream& os, Page<InputIterator> page) {
    for (auto it = page.begin(); it != page.end(); ++it) {
        os << *it;
    }
    return os;
}

// Paginator template implementation

template<typename InputIterator>
Paginator<InputIterator>::Paginator(InputIterator range_begin, InputIterator range_end, std::size_t page_size) noexcept
        : range_begin_(range_begin)
        , range_end_(range_end)
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