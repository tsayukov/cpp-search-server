#pragma once

#include "search_server/search_server.hpp"

#include <string>
#include <string_view>
#include <vector>

void AddDocument(SearchServer& server, int document_id, std::string_view document, DocumentStatus status,
                 const std::vector<int>& ratings);

void RemoveDocument(SearchServer& server, int document_id);

std::vector<Document> FindTopDocuments(const SearchServer& server, std::string_view raw_query);

std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const SearchServer& server,
                                                                        std::string_view raw_query,
                                                                        int document_id);

const std::map<std::string_view, double>& GetWordFrequencies(const SearchServer& server, int document_id);

void RemoveDuplicatesWithProfiling(SearchServer& server);
