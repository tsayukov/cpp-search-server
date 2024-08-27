#include <search_server/document.hpp>

Document::Document(int id, double relevance, int rating) noexcept
        : id(id)
        , rating(rating)
        , relevance(relevance) {
}

std::ostream& operator<<(std::ostream& output, const Document& document) {
    using namespace std::string_literals;
    return output << "{ "s
                  << "document_id = "s << document.id << ", "s
                  << "relevance = "s << document.relevance << ", "s
                  << "rating = "s << document.rating
                  << " }"s;
}
