#include "document.h"

using std::operator""s;

Document::Document(int id, double relevance, int rating) noexcept
        : id(id)
        , relevance(relevance)
        , rating(rating) {
}

std::ostream& operator<<(std::ostream& os, const Document& document) {
    return os << "{ "s
              << "document_id = "s << document.id << ", "s
              << "relevance = "s << document.relevance << ", "s
              << "rating = "s << document.rating
              << " }"s;
}