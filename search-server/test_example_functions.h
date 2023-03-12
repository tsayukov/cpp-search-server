#pragma once

#include <string>
#include <vector>

#include "remove_duplicates.h"
#include "search_server.h"

void AddDocument(SearchServer& server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings);

void RemoveDocument(SearchServer& server, int document_id);

std::vector<Document> FindTopDocuments(const SearchServer& server, const std::string& raw_query);

std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const SearchServer& server,
                                                                   const std::string& raw_query,
                                                                   int document_id);

const std::map<std::string, double>& GetWordFrequencies(const SearchServer& server, int document_id);

void RemoveDuplicatesWithProfiling(SearchServer& server);