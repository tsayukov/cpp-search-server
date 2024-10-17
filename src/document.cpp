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
