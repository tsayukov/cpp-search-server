/// \file document.cpp
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \author Yandex Practicum https://practicum.yandex.ru/
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#include <search_server/document.hpp>

namespace search_server {

Document::Document(int id, double relevance, int rating) noexcept
        : id(id)
        , rating(rating)
        , relevance(relevance) {}

std::ostream& operator<<(std::ostream& output, const Document& document) {
    // clang-format off
    return output << "{ " << "documentId = " << document.id
                  << ", " << "relevance = " << document.relevance
                  << ", " << "rating = " << document.rating
                  << " }";
    // clang-format on
}

} // namespace search_server
