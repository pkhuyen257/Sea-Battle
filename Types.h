#ifndef TYPES_H
#define TYPES_H

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

struct Position {
    int row = -1;
    int col = -1;

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }

    bool operator<(const Position& other) const {
        if (row != other.row) {
            return row < other.row;
        }
        return col < other.col;
    }
};

inline int manhattanDistance(const Position& lhs, const Position& rhs) {
    return std::abs(lhs.row - rhs.row) + std::abs(lhs.col - rhs.col);
}

inline std::string trim(const std::string& input) {
    std::size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }

    std::size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }

    return input.substr(start, end - start);
}

inline std::string toUpper(std::string input) {
    std::transform(input.begin(), input.end(), input.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return input;
}

inline std::string toCoordinate(const Position& position) {
    if (position.row < 0 || position.col < 0) {
        return "??";
    }

    std::ostringstream output;
    output << static_cast<char>('A' + position.col) << (position.row + 1);
    return output.str();
}

inline bool parseCoordinate(const std::string& rawInput, Position& outPosition) {
    const std::string input = toUpper(trim(rawInput));
    if (input.size() < 2 || input.size() > 3) {
        return false;
    }

    const char columnChar = input[0];
    if (columnChar < 'A' || columnChar > 'J') {
        return false;
    }

    const std::string rowPart = input.substr(1);
    for (char ch : rowPart) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }

    const int row = std::stoi(rowPart);
    if (row < 1 || row > 10) {
        return false;
    }

    outPosition.row = row - 1;
    outPosition.col = columnChar - 'A';
    return true;
}

inline std::vector<Position> getArea(const Position& center, int radius, int rows = 10, int cols = 10) {
    std::vector<Position> positions;
    for (int row = std::max(0, center.row - radius); row <= std::min(rows - 1, center.row + radius); ++row) {
        for (int col = std::max(0, center.col - radius); col <= std::min(cols - 1, center.col + radius); ++col) {
            positions.push_back({row, col});
        }
    }
    return positions;
}

#endif
