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

/// \brief Document's current status.
enum class SEARCH_SERVER_EXPORT DocumentStatus {
    kActual,     ///< Actual Document
    kIrrelevant, ///< Irrelevant Document
    kBanned,     ///< Banned Document
    kRemoved,    ///< Removed Document
};

/// \brief Document's ranking properties.
///
/// Each document has an `id` identifier, `relevance`, and `rating` that is the arithmetic mean
/// of all its ratings. Default-initialization of `Document` causes zero-initialization of all its
/// non-static members.
///
struct SEARCH_SERVER_EXPORT Document {
    int id;
    int rating;
    double relevance;

    Document(int id = 0, double relevance = 0.0, int rating = 0) noexcept;
};

/// \brief Insert a textual representation of `document` into `output`.
///
/// The output format of the document is as follows:
/// \code
/// { documentId = <id>, relevance = <relevance>, rating = <rating> }
/// \endcode
///
SEARCH_SERVER_EXPORT
std::ostream& operator<<(std::ostream& output, const Document& document);

} // namespace search_server

#endif // SEARCH_SERVER_DOCUMENT_HPP_
