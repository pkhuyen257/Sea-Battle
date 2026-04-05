#ifndef ENEMYMANAGER_H
#define ENEMYMANAGER_H

#include "Ship.h"

#include <deque>
#include <iosfwd>
#include <random>
#include <string>
#include <vector>

class EnemyManager {
public:
    void spawnEnemies(std::mt19937& randomEngine, const std::vector<Position>& blockedCells);

    std::vector<Ship>& enemies();
    const std::vector<Ship>& enemies() const;

    Ship* getEnemyAt(const Position& position);
    const Ship* getEnemyAt(const Position& position) const;
    bool hasLivingEnemyAt(const Position& position) const;
    bool allDestroyed() const;

    bool damageEnemyAt(const Position& position, int damage, std::deque<std::string>& log, int& score);
    std::vector<Position> scanArea(const Position& center, int radius, const std::string& weather, std::mt19937& randomEngine) const;
    void updateEnemies(Ship& destroyer,
                       Ship& radarShip,
                       Ship& convoy,
                       std::vector<Position>& mines,
                       std::deque<std::string>& log,
                       std::mt19937& randomEngine,
                       const std::string& weather);

    void serialize(std::ostream& output) const;
    static EnemyManager deserialize(std::istream& input);

private:
    std::vector<Ship> enemies_;

    Position chooseTargetCell(const Ship& destroyer, const Ship& radarShip, const Ship& convoy, const Ship& enemy) const;
    Ship* chooseVisibleTarget(Ship& destroyer, Ship& radarShip, Ship& convoy, const Ship& enemy, std::string& targetLabel) const;
    int spottingRangeFor(const Ship& ship) const;
    Position patrolStep(const Position& current, std::size_t enemyIndex) const;
    Position stepTowards(const Position& current, const Position& target) const;
};

#endif
