/// \file document.hpp
/// \brief Document's properties related to its ranking and current status

#ifndef SEARCH_SERVER_DOCUMENT_HPP_
#define SEARCH_SERVER_DOCUMENT_HPP_

#include <search_server/search_server_export.hpp>

#include <ostream>

namespace search_server {

/// Document's current status
enum class SEARCH_SERVER_EXPORT DocumentStatus {
    kActual,
    kIrrelevant,
    kBanned,
    kRemoved,
};

/// Document's ranking properties.
///
/// Each document has an \c id identifier, \c relevance, and \c rating that is
/// the arithmetic mean of all its ratings.
struct SEARCH_SERVER_EXPORT Document {
    int id;
    int rating;
    double relevance;

    Document(int id = 0, double relevance = 0.0, int rating = 0) noexcept;
};

SEARCH_SERVER_EXPORT
std::ostream& operator<<(std::ostream& output, const Document& document);

} // namespace search_server

#endif // SEARCH_SERVER_DOCUMENT_HPP_
