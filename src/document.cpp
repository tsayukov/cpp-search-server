#include <search_server/document.hpp>

Document::Document(int id, double relevance, int rating) noexcept
        : id(id)
        , rating(rating)
        , relevance(relevance) {}

std::ostream& operator<<(std::ostream& output, const Document& document) {
    return output
            << "{ " << "document_id = " << document.id
            << ", " << "relevance = " << document.relevance
            << ", " << "rating = " << document.rating
            << " }";
}
