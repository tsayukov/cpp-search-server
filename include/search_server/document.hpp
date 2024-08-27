/// \file document.hpp
/// \brief Document's properties related to its ranking and current status

#ifndef SEARCH_SERVER_DOCUMENT_HPP
#define SEARCH_SERVER_DOCUMENT_HPP

#include <ostream>

/// Document's current status
enum class DocumentStatus {
    kActual,
    kIrrelevant,
    kBanned,
    kRemoved,
};

/// Document's ranking properties.
///
/// Each document has an \c id identifier, \c relevance, and \c rating that is
/// the arithmetic mean of all its ratings.
struct Document {
    int id;
    int rating;
    double relevance;

    Document(int id = 0, double relevance = 0.0, int rating = 0) noexcept;
};

std::ostream& operator<<(std::ostream& output, const Document& document);

#endif // SEARCH_SERVER_DOCUMENT_HPP
