/// \file document.hpp
/// \brief Document's properties related to its ranking and current status
/// \author Pavel Tsayukov https://github.com/tsayukov/cpp-search-server
/// \author Yandex Practicum https://practicum.yandex.ru/
/// \copyright CC BY-NC 4.0 https://creativecommons.org/licenses/by-nc/4.0/

#ifndef SEARCH_SERVER_DOCUMENT_HPP_
#define SEARCH_SERVER_DOCUMENT_HPP_

#include <search_server/export.hpp>

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

/// Insert a textual representation of \c document into \c output as follows:
/// \code
/// { documentId = <id>, relevance = <relevance>, rating = <rating> }
/// \endcode
SEARCH_SERVER_EXPORT
std::ostream& operator<<(std::ostream& output, const Document& document);

} // namespace search_server

#endif // SEARCH_SERVER_DOCUMENT_HPP_
