#include "Board.h"

#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>
#include <string>

Board::Board(int rows, int cols) : rows_(rows), cols_(cols), grid_(rows, std::vector<char>(cols, '~')) {
}

void Board::initializeGrid() {
    for (std::vector<char>& row : grid_) {
        std::fill(row.begin(), row.end(), '~');
    }
}

bool Board::isValidCoordinate(const Position& position) const {
    return position.row >= 0 && position.row < rows_ && position.col >= 0 && position.col < cols_;
}

char Board::getCell(const Position& position) const {
    if (!isValidCoordinate(position)) {
        return '#';
    }
    return grid_[position.row][position.col];
}

void Board::setCell(const Position& position, char symbol) {
    if (isValidCoordinate(position)) {
        grid_[position.row][position.col] = symbol;
    }
}

void Board::markHit(const Position& position) {
    setCell(position, 'X');
}

void Board::markMiss(const Position& position) {
    if (getCell(position) != 'X') {
        setCell(position, 'o');
    }
}

void Board::clearTransientSignals() {
    for (std::vector<char>& row : grid_) {
        for (char& cell : row) {
            if (cell == '?') {
                cell = '~';
            }
        }
    }
}

void Board::clearSymbol(char symbol) {
    for (std::vector<char>& row : grid_) {
        for (char& cell : row) {
            if (cell == symbol) {
                cell = '~';
            }
        }
    }
}

bool Board::canPlace(const std::vector<Position>& cells, const std::vector<Position>& blockedCells) const {
    for (const Position& cell : cells) {
        if (!isValidCoordinate(cell)) {
            return false;
        }
        if (std::find(blockedCells.begin(), blockedCells.end(), cell) != blockedCells.end()) {
            return false;
        }
    }
    return true;
}

bool Board::moveShip(Ship& ship, int dRow, int dCol, const std::vector<Position>& blockedCells) const {
    std::vector<Position> updatedCells = ship.getOccupiedCells();
    for (Position& cell : updatedCells) {
        cell.row += dRow;
        cell.col += dCol;
    }

    if (!canPlace(updatedCells, blockedCells)) {
        return false;
    }

    ship.setOccupiedCells(updatedCells);
    return true;
}

std::vector<std::string> Board::renderEnemyBoard() const {
    std::vector<std::string> lines;
    std::string header = "    ";
    for (int col = 0; col < cols_; ++col) {
        header += static_cast<char>('A' + col);
        header += ' ';
    }
    lines.push_back(header);

    for (int row = 0; row < rows_; ++row) {
        std::string line = (row + 1 < 10 ? " " : "");
        line += std::to_string(row + 1);
        line += "  ";
        for (int col = 0; col < cols_; ++col) {
            line += grid_[row][col];
            line += ' ';
        }
        lines.push_back(line);
    }
    return lines;
}

std::vector<std::string> Board::renderFriendlyBoard(const std::vector<Ship>& ships) const {
    std::vector<std::vector<char>> display(rows_, std::vector<char>(cols_, '~'));
    for (const Ship& ship : ships) {
        if (!ship.isAlive()) {
            continue;
        }
        for (const Position& position : ship.getOccupiedCells()) {
            if (isValidCoordinate(position)) {
                display[position.row][position.col] = ship.symbol();
            }
        }
    }

    std::vector<std::string> lines;
    std::string header = "    ";
    for (int col = 0; col < cols_; ++col) {
        header += static_cast<char>('A' + col);
        header += ' ';
    }
    lines.push_back(header);

    for (int row = 0; row < rows_; ++row) {
        std::string line = (row + 1 < 10 ? " " : "");
        line += std::to_string(row + 1);
        line += "  ";
        for (int col = 0; col < cols_; ++col) {
            line += display[row][col];
            line += ' ';
        }
        lines.push_back(line);
    }
    return lines;
}

int Board::rows() const {
    return rows_;
}

int Board::cols() const {
    return cols_;
}

void Board::serialize(std::ostream& output) const {
    output << rows_ << ' ' << cols_ << '\n';
    for (const std::vector<char>& row : grid_) {
        for (char cell : row) {
            output << cell;
        }
        output << '\n';
    }
}

Board Board::deserialize(std::istream& input) {
    int rows = 10;
    int cols = 10;
    input >> rows >> cols;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Board board(rows, cols);
    for (int row = 0; row < rows; ++row) {
        std::string line;
        std::getline(input, line);
        if (static_cast<int>(line.size()) < cols) {
            line.append(cols - line.size(), '~');
        }
        for (int col = 0; col < cols; ++col) {
            board.grid_[row][col] = line[col];
        }
    }
    return board;
}
