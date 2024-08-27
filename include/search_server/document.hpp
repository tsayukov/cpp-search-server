/// \file document.hpp
/// \brief Document's properties related to its ranking and current status

#ifndef SEARCH_SERVER_DOCUMENT_HPP
#define SEARCH_SERVER_DOCUMENT_HPP

#include <ostream>

/// Document's current status
enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

/// Document's ranking properties.
///
/// Each document has an \c id identifier, \c rating that is the arithmetic
/// mean of all its ratings, and \c relevance.
struct Document {
    Document() noexcept = default;

    Document(int id, double relevance, int rating) noexcept;

    int id = 0;
    int rating = 0;
    double relevance = 0.0;
};

std::ostream& operator<<(std::ostream& output, const Document& document);

#endif // SEARCH_SERVER_DOCUMENT_HPP
