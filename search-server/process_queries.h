#pragma once

#include "document.h"
#include "flatten_container.h"
#include "search_server.h"

#include <vector>

std::vector<std::vector<Document>> ProcessQueries(
        const SearchServer& search_server,
        const std::vector<std::string>& queries);

auto ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries)
        -> decltype(MakeFlattenContainer(ProcessQueries(search_server, queries)));