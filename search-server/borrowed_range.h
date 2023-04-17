#pragma once

template<typename T>
class BorrowedRange {
public:
    BorrowedRange(T begin, T end) noexcept(noexcept(T(begin)) && noexcept(T(end)))
            : begin_(begin)
            , end_(end) {
    }

    [[nodiscard]] T begin() const noexcept(noexcept(T(begin_))) {
        return begin_;
    }

    [[nodiscard]] T end() const noexcept(noexcept(T(end_))) {
        return end_;
    }

private:
    T begin_;
    T end_;
};