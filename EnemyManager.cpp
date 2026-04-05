#include "EnemyManager.h"

#include <algorithm>
#include <istream>
#include <limits>
#include <ostream>

namespace {
bool containsPosition(const std::vector<Position>& positions, const Position& target) {
    return std::find(positions.begin(), positions.end(), target) != positions.end();
}

int attackRangeFor(const Ship& ship) {
    switch (ship.role()) {
    case ShipRole::PatrolBoat:
        return 2;
    case ShipRole::Raider:
        return 3;
    case ShipRole::Submarine:
        return 4;
    default:
        return 1;
    }
}

int damageFor(const Ship& ship) {
    switch (ship.role()) {
    case ShipRole::Raider:
        return 2;
    case ShipRole::Submarine:
        return 2;
    default:
        return 1;
    }
}

int closestDistanceToShip(const Position& from, const Ship& ship) {
    int distance = 1000;
    for (const Position& cell : ship.getOccupiedCells()) {
        distance = std::min(distance, manhattanDistance(from, cell));
    }
    return distance;
}
} // namespace

void EnemyManager::spawnEnemies(std::mt19937& randomEngine, const std::vector<Position>& blockedCells) {
    enemies_.clear();

    std::uniform_int_distribution<int> rowDistribution(0, 9);
    std::uniform_int_distribution<int> colDistribution(5, 9);

    auto isBlocked = [&](const Position& position) {
        if (containsPosition(blockedCells, position)) {
            return true;
        }
        for (const Ship& enemy : enemies_) {
            if (enemy.occupies(position)) {
                return true;
            }
        }
        return false;
    };

    auto randomFreeCell = [&]() {
        Position position;
        do {
            position = {rowDistribution(randomEngine), colDistribution(randomEngine)};
        } while (isBlocked(position));
        return position;
    };

    enemies_.push_back(Ship::createPatrolBoat(randomFreeCell()));
    enemies_.push_back(Ship::createPatrolBoat(randomFreeCell()));
    enemies_.push_back(Ship::createRaider(randomFreeCell()));
    enemies_.push_back(Ship::createSubmarine(randomFreeCell()));
}

std::vector<Ship>& EnemyManager::enemies() {
    return enemies_;
}

const std::vector<Ship>& EnemyManager::enemies() const {
    return enemies_;
}

Ship* EnemyManager::getEnemyAt(const Position& position) {
    for (Ship& enemy : enemies_) {
        if (enemy.isAlive() && enemy.occupies(position)) {
            return &enemy;
        }
    }
    return nullptr;
}

const Ship* EnemyManager::getEnemyAt(const Position& position) const {
    for (const Ship& enemy : enemies_) {
        if (enemy.isAlive() && enemy.occupies(position)) {
            return &enemy;
        }
    }
    return nullptr;
}

bool EnemyManager::hasLivingEnemyAt(const Position& position) const {
    return getEnemyAt(position) != nullptr;
}

bool EnemyManager::allDestroyed() const {
    for (const Ship& enemy : enemies_) {
        if (enemy.isAlive()) {
            return false;
        }
    }
    return true;
}

bool EnemyManager::damageEnemyAt(const Position& position, int damage, std::deque<std::string>& log, int& score) {
    Ship* enemy = getEnemyAt(position);
    if (enemy == nullptr) {
        return false;
    }

    enemy->setRevealed(true);
    enemy->takeDamage(damage);
    if (enemy->isAlive()) {
        log.push_front(enemy->name() + " hit at " + toCoordinate(position) + ".");
        score += 20;
    } else {
        log.push_front(enemy->name() + " destroyed at " + toCoordinate(position) + ".");
        score += 50;
    }

    while (log.size() > 5) {
        log.pop_back();
    }
    return true;
}

std::vector<Position> EnemyManager::scanArea(const Position& center, int radius, const std::string& weather, std::mt19937& randomEngine) const {
    std::vector<Position> scanCells = getArea(center, radius);
    std::vector<Position> signals;
    std::uniform_int_distribution<int> offsetDistribution(-1, 1);

    for (const Ship& enemy : enemies_) {
        if (!enemy.isAlive()) {
            continue;
        }

        const Position enemyPosition = enemy.getOccupiedCells().front();
        if (std::find(scanCells.begin(), scanCells.end(), enemyPosition) == scanCells.end()) {
            continue;
        }

        Position signal = enemyPosition;
        if (enemy.isStealth() || weather == "Fog") {
            const Position shifted{enemyPosition.row + offsetDistribution(randomEngine),
                                   enemyPosition.col + offsetDistribution(randomEngine)};
            if (shifted.row >= 0 && shifted.row < 10 && shifted.col >= 0 && shifted.col < 10) {
                signal = shifted;
            }
        }

        if (std::find(signals.begin(), signals.end(), signal) == signals.end()) {
            signals.push_back(signal);
        }
    }

    if (signals.empty() && weather != "Clear") {
        std::uniform_int_distribution<int> falseSignalIndex(0, static_cast<int>(scanCells.size()) - 1);
        signals.push_back(scanCells[falseSignalIndex(randomEngine)]);
    }

    return signals;
}

void EnemyManager::updateEnemies(Ship& destroyer,
                                 Ship& radarShip,
                                 Ship& convoy,
                                 std::vector<Position>& mines,
                                 std::deque<std::string>& log,
                                 std::mt19937& randomEngine,
                                 const std::string& weather) {
    std::uniform_int_distribution<int> chanceDistribution(1, 100);

    auto trimLog = [&]() {
        while (log.size() > 5) {
            log.pop_back();
        }
    };

    auto maybeDetonateMine = [&](Ship& enemy) {
        if (!enemy.isAlive()) {
            return;
        }

        const Position position = enemy.getOccupiedCells().front();
        auto mine = std::find(mines.begin(), mines.end(), position);
        if (mine == mines.end()) {
            return;
        }

        enemy.takeDamage(2);
        if (enemy.isAlive()) {
            log.push_front("A mine explodes under " + enemy.name() + " at " + toCoordinate(position) + ".");
        } else {
            enemy.setRevealed(true);
            log.push_front("Minefield destroys " + enemy.name() + " at " + toCoordinate(position) + ".");
        }
        mines.erase(mine);
        trimLog();
    };

    for (std::size_t index = 0; index < enemies_.size(); ++index) {
        Ship& enemy = enemies_[index];
        if (!enemy.isAlive()) {
            continue;
        }

        int attackRange = attackRangeFor(enemy);
        int hitChance = 70;
        if (weather == "Fog") {
            hitChance -= 10;
        } else if (weather == "Storm") {
            hitChance -= 20;
        }

        std::vector<std::pair<Ship*, std::string>> targets;
        if (convoy.isAlive()) {
            targets.push_back({&convoy, "convoy"});
        }
        if (destroyer.isAlive()) {
            targets.push_back({&destroyer, "destroyer"});
        }
        if (radarShip.isAlive()) {
            targets.push_back({&radarShip, "radar ship"});
        }

        std::string targetLabel;
        Ship* visibleTarget = chooseVisibleTarget(destroyer, radarShip, convoy, enemy, targetLabel);

        Ship* selectedTarget = nullptr;
        if (visibleTarget != nullptr &&
            closestDistanceToShip(enemy.getOccupiedCells().front(), *visibleTarget) <= attackRange) {
            selectedTarget = visibleTarget;
        }

        if (selectedTarget == nullptr) {
            const Position current = enemy.getOccupiedCells().front();
            const Position target = visibleTarget != nullptr
                                        ? chooseTargetCell(destroyer, radarShip, convoy, enemy)
                                        : patrolStep(current, index);
            const Position next = stepTowards(current, target);

            bool occupiedByEnemy = false;
            for (std::size_t otherIndex = 0; otherIndex < enemies_.size(); ++otherIndex) {
                if (index == otherIndex || !enemies_[otherIndex].isAlive()) {
                    continue;
                }
                if (enemies_[otherIndex].occupies(next)) {
                    occupiedByEnemy = true;
                    break;
                }
            }

            if (!occupiedByEnemy && next != current) {
                enemy.setOccupiedCells({next});
                maybeDetonateMine(enemy);
            }
            continue;
        }

        if (targetLabel == "convoy" && destroyer.isAlive()) {
            for (const Position& escortCell : destroyer.getOccupiedCells()) {
                for (const Position& convoyCell : convoy.getOccupiedCells()) {
                    if (manhattanDistance(escortCell, convoyCell) <= 1) {
                        hitChance -= 15;
                        break;
                    }
                }
            }
        }

        if (chanceDistribution(randomEngine) <= hitChance) {
            selectedTarget->takeDamage(damageFor(enemy));
            log.push_front(enemy.name() + " hits the " + targetLabel + ".");
        } else {
            log.push_front(enemy.name() + " misses the " + targetLabel + ".");
        }
        trimLog();
    }
}

void EnemyManager::serialize(std::ostream& output) const {
    output << enemies_.size() << '\n';
    for (const Ship& enemy : enemies_) {
        enemy.serialize(output);
    }
}

EnemyManager EnemyManager::deserialize(std::istream& input) {
    EnemyManager manager;
    std::size_t count = 0;
    input >> count;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    for (std::size_t index = 0; index < count; ++index) {
        manager.enemies_.push_back(Ship::deserialize(input));
    }
    return manager;
}

Position EnemyManager::chooseTargetCell(const Ship& destroyer, const Ship& radarShip, const Ship& convoy, const Ship& enemy) const {
    Position bestTarget = convoy.getOccupiedCells().empty() ? Position{4, 4} : convoy.getOccupiedCells().front();
    int bestDistance = 1000;

    auto consider = [&](const Ship& ship) {
        if (!ship.isAlive()) {
            return;
        }
        for (const Position& cell : ship.getOccupiedCells()) {
            const int distance = manhattanDistance(enemy.getOccupiedCells().front(), cell);
            if (distance < bestDistance) {
                bestDistance = distance;
                bestTarget = cell;
            }
        }
    };

    consider(convoy);
    consider(destroyer);
    consider(radarShip);
    return bestTarget;
}

Ship* EnemyManager::chooseVisibleTarget(Ship& destroyer,
                                        Ship& radarShip,
                                        Ship& convoy,
                                        const Ship& enemy,
                                        std::string& targetLabel) const {
    Ship* bestTarget = nullptr;
    int bestDistance = 1000;
    const Position current = enemy.getOccupiedCells().front();
    const int spottingRange = spottingRangeFor(enemy);

    auto consider = [&](Ship& ship, const std::string& label) {
        if (!ship.isAlive()) {
            return;
        }

        const int distance = closestDistanceToShip(current, ship);
        if (distance <= spottingRange && distance < bestDistance) {
            bestDistance = distance;
            bestTarget = &ship;
            targetLabel = label;
        }
    };

    consider(convoy, "convoy");
    consider(destroyer, "destroyer");
    consider(radarShip, "radar ship");
    return bestTarget;
}

int EnemyManager::spottingRangeFor(const Ship& ship) const {
    switch (ship.role()) {
    case ShipRole::PatrolBoat:
        return 3;
    case ShipRole::Raider:
        return 4;
    case ShipRole::Submarine:
        return 3;
    default:
        return 3;
    }
}

Position EnemyManager::patrolStep(const Position& current, std::size_t enemyIndex) const {
    Position next = current;

    if (current.col > 0) {
        --next.col;
        return next;
    }

    if (enemyIndex % 2 == 0 && current.row < 9) {
        ++next.row;
    } else if (enemyIndex % 2 == 1 && current.row > 0) {
        --next.row;
    }

    return next;
}

Position EnemyManager::stepTowards(const Position& current, const Position& target) const {
    Position next = current;
    if (target.row < current.row) {
        --next.row;
    } else if (target.row > current.row) {
        ++next.row;
    } else if (target.col < current.col) {
        --next.col;
    } else if (target.col > current.col) {
        ++next.col;
    }

    next.row = std::max(0, std::min(9, next.row));
    next.col = std::max(0, std::min(9, next.col));
    return next;
}
