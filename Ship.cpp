#include "Ship.h"

#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>
#include <utility>

Ship::Ship()
    : role_(ShipRole::Destroyer),
      symbol_('D'),
      maxHp_(0),
      hp_(0),
      movable_(false),
      revealed_(false) {
}

Ship::Ship(ShipRole role, std::string name, char symbol, int maxHp, bool movable, std::vector<Position> positions)
    : role_(role),
      name_(std::move(name)),
      symbol_(symbol),
      maxHp_(maxHp),
      hp_(maxHp),
      movable_(movable),
      revealed_(role == ShipRole::Destroyer || role == ShipRole::RadarShip || role == ShipRole::Convoy),
      positions_(std::move(positions)) {
}

Ship Ship::createDestroyer(const Position& position) {
    return Ship(ShipRole::Destroyer, "Destroyer", 'D', 5, true, {position});
}

Ship Ship::createRadarShip(const Position& position) {
    return Ship(ShipRole::RadarShip, "Radar Ship", 'R', 4, true, {position});
}

Ship Ship::createConvoy(const Position& leftmostPosition) {
    return Ship(ShipRole::Convoy, "Convoy", 'C', 6, false,
                {{leftmostPosition.row, leftmostPosition.col},
                 {leftmostPosition.row, leftmostPosition.col + 1},
                 {leftmostPosition.row, leftmostPosition.col + 2}});
}

Ship Ship::createPatrolBoat(const Position& position) {
    return Ship(ShipRole::PatrolBoat, "Patrol Boat", 'P', 2, true, {position});
}

Ship Ship::createRaider(const Position& position) {
    return Ship(ShipRole::Raider, "Raider", 'A', 3, true, {position});
}

Ship Ship::createSubmarine(const Position& position) {
    return Ship(ShipRole::Submarine, "Submarine", 'S', 2, true, {position});
}

ShipRole Ship::role() const {
    return role_;
}

const std::string& Ship::name() const {
    return name_;
}

char Ship::symbol() const {
    return symbol_;
}

int Ship::hp() const {
    return hp_;
}

int Ship::maxHp() const {
    return maxHp_;
}

bool Ship::isAlive() const {
    return hp_ > 0;
}

bool Ship::canMove() const {
    return movable_ && isAlive();
}

bool Ship::isRevealed() const {
    return revealed_;
}

bool Ship::isStealth() const {
    return role_ == ShipRole::Submarine;
}

void Ship::setRevealed(bool revealed) {
    revealed_ = revealed;
}

void Ship::takeDamage(int amount) {
    hp_ = std::max(0, hp_ - std::max(0, amount));
}

void Ship::heal(int amount) {
    hp_ = std::min(maxHp_, hp_ + std::max(0, amount));
}

const std::vector<Position>& Ship::getOccupiedCells() const {
    return positions_;
}

void Ship::setOccupiedCells(const std::vector<Position>& positions) {
    positions_ = positions;
}

void Ship::translate(int dRow, int dCol) {
    for (Position& position : positions_) {
        position.row += dRow;
        position.col += dCol;
    }
}

bool Ship::occupies(const Position& position) const {
    return std::find(positions_.begin(), positions_.end(), position) != positions_.end();
}

void Ship::serialize(std::ostream& output) const {
    output << static_cast<int>(role_) << '\n';
    output << name_ << '\n';
    output << symbol_ << ' ' << maxHp_ << ' ' << hp_ << ' ' << movable_ << ' ' << revealed_ << ' ' << positions_.size() << '\n';
    for (const Position& position : positions_) {
        output << position.row << ' ' << position.col << '\n';
    }
}

Ship Ship::deserialize(std::istream& input) {
    int roleValue = 0;
    input >> roleValue;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string name;
    std::getline(input, name);

    char symbol = '~';
    int maxHp = 0;
    int hp = 0;
    bool movable = false;
    bool revealed = false;
    std::size_t positionCount = 0;
    input >> symbol >> maxHp >> hp >> movable >> revealed >> positionCount;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::vector<Position> positions;
    for (std::size_t index = 0; index < positionCount; ++index) {
        Position position;
        input >> position.row >> position.col;
        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        positions.push_back(position);
    }

    Ship ship(static_cast<ShipRole>(roleValue), name, symbol, maxHp, movable, positions);
    ship.hp_ = hp;
    ship.revealed_ = revealed;
    return ship;
}
