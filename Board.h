#ifndef BOARD_H
#define BOARD_H

#include "Ship.h"

#include <iosfwd>
#include <vector>

class Board {
public:
    Board(int rows = 10, int cols = 10);

    void initializeGrid();
    bool isValidCoordinate(const Position& position) const;
    char getCell(const Position& position) const;
    void setCell(const Position& position, char symbol);
    void markHit(const Position& position);
    void markMiss(const Position& position);
    void clearTransientSignals();
    void clearSymbol(char symbol);

    bool canPlace(const std::vector<Position>& cells, const std::vector<Position>& blockedCells) const;
    bool moveShip(Ship& ship, int dRow, int dCol, const std::vector<Position>& blockedCells) const;

    std::vector<std::string> renderEnemyBoard() const;
    std::vector<std::string> renderFriendlyBoard(const std::vector<Ship>& ships) const;

    int rows() const;
    int cols() const;

    void serialize(std::ostream& output) const;
    static Board deserialize(std::istream& input);

private:
    int rows_;
    int cols_;
    std::vector<std::vector<char>> grid_;
};

#endif
