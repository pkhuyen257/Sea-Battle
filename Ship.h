#ifndef SHIP_H
#define SHIP_H

#include "Types.h"

#include <iosfwd>
#include <string>
#include <vector>

enum class ShipRole {
    Destroyer = 0,
    RadarShip = 1,
    Convoy = 2,
    PatrolBoat = 3,
    Raider = 4,
    Submarine = 5
};

class Ship {
public:
    Ship();
    Ship(ShipRole role, std::string name, char symbol, int maxHp, bool movable, std::vector<Position> positions);

    static Ship createDestroyer(const Position& position);
    static Ship createRadarShip(const Position& position);
    static Ship createConvoy(const Position& leftmostPosition);
    static Ship createPatrolBoat(const Position& position);
    static Ship createRaider(const Position& position);
    static Ship createSubmarine(const Position& position);

    ShipRole role() const;
    const std::string& name() const;
    char symbol() const;
    int hp() const;
    int maxHp() const;
    bool isAlive() const;
    bool canMove() const;
    bool isRevealed() const;
    bool isStealth() const;
    void setRevealed(bool revealed);

    void takeDamage(int amount);
    void heal(int amount);

    const std::vector<Position>& getOccupiedCells() const;
    void setOccupiedCells(const std::vector<Position>& positions);
    void translate(int dRow, int dCol);
    bool occupies(const Position& position) const;

    void serialize(std::ostream& output) const;
    static Ship deserialize(std::istream& input);

private:
    ShipRole role_;
    std::string name_;
    char symbol_;
    int maxHp_;
    int hp_;
    bool movable_;
    bool revealed_;
    std::vector<Position> positions_;
};

#endif
