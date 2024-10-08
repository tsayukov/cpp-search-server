#pragma once

#include <ostream>

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    Document() noexcept = default;

    Document(int id, double relevance, int rating) noexcept;

    int id = 0;
    int rating = 0;
    double relevance = 0.0;
};

std::ostream& operator<<(std::ostream& output, const Document& document);