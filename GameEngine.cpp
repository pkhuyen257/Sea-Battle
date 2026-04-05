#include "GameEngine.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>

namespace {
std::string readLine() {
    std::string input;
    std::getline(std::cin, input);
    return trim(input);
}
} // namespace

GameEngine::GameEngine()
    : randomEngine_(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count())),
      turnNumber_(1),
      score_(0),
      convoyEscaped_(false),
      weather_("Clear") {
}

void GameEngine::run() {
    bool shouldQuit = false;
    while (!shouldQuit) {
        uiRenderer_.showMainMenu();
        uiRenderer_.showMessage("Select an option:", MessageTone::Info);
        const std::string choice = readLine();

        if (choice == "1") {
            startNewGame();
            uiRenderer_.showMissionBriefing(mission_, friendlyShips(), currentThreatLevel());
            pauseForEnter();
            playActiveMission();
        } else if (choice == "2") {
            uiRenderer_.showActionScreen("LOAD GAME", {"Loading your saved game from savegame.txt"});
            uiRenderer_.showLoading("Loading saved game", MessageTone::Info, 4, 120);
            if (loadGame()) {
                playActiveMission();
            } else {
                uiRenderer_.showMessage("No valid save file was found.", MessageTone::Danger);
                pauseForEnter();
            }
        } else if (choice == "3") {
            uiRenderer_.showHelp();
            pauseForEnter();
        } else if (choice == "4") {
            shouldQuit = true;
        } else {
            uiRenderer_.showMessage("Please enter 1, 2, 3, or 4.", MessageTone::Warning);
            pauseForEnter();
        }
    }
}

void GameEngine::startNewGame() {
    enemyBoard_.initializeGrid();
    destroyer_ = Ship::createDestroyer({1, 3});
    radarShip_ = Ship::createRadarShip({2, 4});
    convoy_ = Ship::createConvoy({3, 0});
    mission_ = Mission("Protect the Supply Convoy", "Last 8 turns and escort the convoy to the safe zone.", 8);
    turnNumber_ = 1;
    score_ = 0;
    convoyEscaped_ = false;
    weather_ = "Clear";
    mines_.clear();
    missionLog_.clear();
    enemyManager_.spawnEnemies(randomEngine_, occupiedFriendlyCells(true, true, true));
    updateWeather();
    addLog("Fleet deployed. Escort the convoy through dangerous waters.");
}

bool GameEngine::loadGame() {
    SaveData saveData;
    if (!saveManager_.loadFromFile("savegame.txt", saveData)) {
        return false;
    }

    applySaveData(saveData);
    addLog("Game loaded from save file.");
    return true;
}

void GameEngine::playActiveMission() {
    while (true) {
        refreshRadarVision();
        for (const Position& mine : mines_) {
            enemyBoard_.setCell(mine, 'M');
        }

        uiRenderer_.renderGame(mission_,
                               turnNumber_,
                               weather_,
                               score_,
                               currentThreatLevel(),
                               enemyBoard_,
                               friendlyShips(),
                               missionLog_);

        if (mission_.isComplete(convoyEscaped_)) {
            uiRenderer_.showEndScreen(true,
                                      "The convoy has safely reached the destination.",
                                      score_,
                                      turnNumber_,
                                      enemiesDestroyed());
            pauseForEnter();
            return;
        }

        if (mission_.isFailed(turnNumber_, convoyEscaped_, convoy_, destroyer_, radarShip_)) {
            uiRenderer_.showEndScreen(false,
                                      "The convoy has been lost and the route has fallen to enemy control.",
                                      score_,
                                      turnNumber_,
                                      enemiesDestroyed());
            pauseForEnter();
            return;
        }

        processTurn();
    }
}

void GameEngine::processTurn() {
    enemyBoard_.clearTransientSignals();
    refreshRadarVision();

    bool turnConsumed = false;
    while (!turnConsumed) {
        uiRenderer_.showMessage("Enter choice:", MessageTone::Info);
        const std::string action = readLine();

        if (action == "1") {
            turnConsumed = handleMoveShip();
        } else if (action == "2") {
            turnConsumed = handleRadarScan();
        } else if (action == "3") {
            turnConsumed = handleFireTorpedo();
        } else if (action == "4") {
            turnConsumed = handleDeployMine();
        } else if (action == "5") {
            addLog("Your fleet holds position.");
            turnConsumed = true;
        } else if (action == "6") {
            saveGame();
        } else {
            uiRenderer_.showMessage("Invalid action. Enter a number from 1 to 6.", MessageTone::Warning);
        }
    }

    enemyPhase();
    if (!mission_.isComplete(convoyEscaped_) &&
        !mission_.isFailed(turnNumber_, convoyEscaped_, convoy_, destroyer_, radarShip_)) {
        ++turnNumber_;
        updateWeather();
    }
}

bool GameEngine::handleMoveShip() {
    uiRenderer_.showActionScreen("MOVE",
                                 {"Select ship to move",
                                  "",
                                  "[1] Destroyer",
                                  "[2] Radar Ship"});
    uiRenderer_.showMessage("Choice:", MessageTone::Info);
    const std::string shipChoice = readLine();

    Ship* ship = nullptr;
    std::vector<Position> blocked;
    if (shipChoice == "1") {
        ship = &destroyer_;
        blocked = occupiedFriendlyCells(false, true, true);
    } else if (shipChoice == "2") {
        ship = &radarShip_;
        blocked = occupiedFriendlyCells(true, false, true);
    } else {
        uiRenderer_.showMessage("Invalid ship selection.", MessageTone::Danger);
        return false;
    }

    if (!ship->canMove()) {
        uiRenderer_.showMessage(ship->name() + " cannot move.", MessageTone::Danger);
        return false;
    }

    uiRenderer_.showActionScreen("MOVE",
                                 {"Current position: " + toCoordinate(ship->getOccupiedCells().front()),
                                  "",
                                  "Direction:",
                                  "W - Up",
                                  "A - Left",
                                  "S - Down",
                                  "D - Right"});
    uiRenderer_.showMessage("Move:", MessageTone::Info);
    const std::string direction = toUpper(readLine());

    int dRow = 0;
    int dCol = 0;
    if (direction == "W") {
        dRow = -1;
    } else if (direction == "S") {
        dRow = 1;
    } else if (direction == "A") {
        dCol = -1;
    } else if (direction == "D") {
        dCol = 1;
    } else {
        uiRenderer_.showMessage("Use W, A, S, or D.", MessageTone::Warning);
        return false;
    }

    if (!enemyBoard_.moveShip(*ship, dRow, dCol, blocked)) {
        uiRenderer_.showMessage("Movement blocked by the map boundary or another friendly unit.", MessageTone::Danger);
        return false;
    }

    addLog(ship->name() + " moved to " + toCoordinate(ship->getOccupiedCells().front()) + ".");
    uiRenderer_.showMessage(ship->name() + " moved successfully.", MessageTone::Success);
    return true;
}

bool GameEngine::handleRadarScan() {
    if (!radarShip_.isAlive()) {
        uiRenderer_.showMessage("The radar ship has been destroyed.", MessageTone::Danger);
        return false;
    }

    uiRenderer_.showActionScreen("SCAN",
                                 {"Enter the center coordinate for a 3x3 scan area",
                                  "",
                                  "You can scan any 3x3 square on the board."});
    uiRenderer_.showMessage("Scan location:", MessageTone::Info);
    Position target;
    if (!parseCoordinate(readLine(), target)) {
        uiRenderer_.showMessage("Invalid coordinate.", MessageTone::Danger);
        return false;
    }

    if (!canScanAt(target)) {
        uiRenderer_.showMessage("Target is out of radar range.", MessageTone::Warning);
        return false;
    }

    uiRenderer_.showActionScreen("SCAN",
                                 {"Scan location: " + toCoordinate(target),
                                  "",
                                  "Scanning enemy area..."});
    uiRenderer_.showLoading("Scanning", MessageTone::Info, 4, 160);

    const std::vector<Position> signals = enemyManager_.scanArea(target, 1, weather_, randomEngine_);
    if (signals.empty()) {
        addLog("Radar scan at " + toCoordinate(target) + " found no activity.");
        uiRenderer_.showMessage("No activity detected in the scan zone.", MessageTone::Success);
        return true;
    }

    for (const Position& signal : signals) {
        if (enemyBoard_.getCell(signal) == '~') {
            enemyBoard_.setCell(signal, '?');
        }
    }

    std::ostringstream message;
    message << "Radar reports " << signals.size() << " suspicious contact";
    if (signals.size() > 1) {
        message << "s";
    }
    message << " near " << toCoordinate(target) << ".";
    addLog(message.str());
    uiRenderer_.showAlert("ENEMY FOUND",
                          {"Enemy activity has been found nearby.",
                           "Danger level in this area: " + currentThreatLevel()},
                          MessageTone::Warning);
    return true;
}

bool GameEngine::handleFireTorpedo() {
    if (!destroyer_.isAlive() && !radarShip_.isAlive()) {
        uiRenderer_.showMessage("No ship remains capable of firing torpedoes.", MessageTone::Danger);
        return false;
    }

    uiRenderer_.showActionScreen("ATTACK",
                                 {"Enter target coordinate",
                                  "",
                                  "Destroyer range is slightly longer than before."});
    uiRenderer_.showMessage("Target:", MessageTone::Info);
    Position target;
    if (!parseCoordinate(readLine(), target)) {
        uiRenderer_.showMessage("Invalid coordinate.", MessageTone::Danger);
        return false;
    }

    if (!canUseTorpedoAt(target)) {
        uiRenderer_.showMessage("Target is out of torpedo range.", MessageTone::Warning);
        return false;
    }

    uiRenderer_.showActionScreen("ATTACK",
                                 {"Target: " + toCoordinate(target),
                                  "",
                                  "Launching attack..."});
    uiRenderer_.showLoading("Launching", MessageTone::Danger, 4, 120);

    int hitChance = 85;
    if (weather_ == "Fog") {
        hitChance -= 15;
    } else if (weather_ == "Storm") {
        hitChance -= 25;
    }

    std::uniform_int_distribution<int> chanceDistribution(1, 100);
    if (chanceDistribution(randomEngine_) > hitChance) {
        enemyBoard_.markMiss(target);
        addLog("Torpedo veers off course at " + toCoordinate(target) + ".");
        uiRenderer_.showMessage("MISS. No confirmed enemy contact.", MessageTone::Neutral);
        return true;
    }

    if (enemyManager_.damageEnemyAt(target, 2, missionLog_, score_)) {
        enemyBoard_.markHit(target);
        uiRenderer_.showMessage("DIRECT HIT! Enemy vessel damaged.", MessageTone::Danger);
    } else {
        enemyBoard_.markMiss(target);
        addLog("Torpedo misses at " + toCoordinate(target) + ".");
        uiRenderer_.showMessage("MISS. No enemy vessel detected.", MessageTone::Neutral);
    }
    return true;
}

bool GameEngine::handleDeployMine() {
    if (!destroyer_.isAlive()) {
        uiRenderer_.showMessage("The destroyer is unavailable to deploy mines.", MessageTone::Danger);
        return false;
    }

    uiRenderer_.showActionScreen("PLACE MINE",
                                 {"Select coordinate to place defensive mine",
                                  "",
                                  "Mine range is 2 cells from the destroyer."});
    uiRenderer_.showMessage("Coordinate:", MessageTone::Special);
    Position target;
    if (!parseCoordinate(readLine(), target)) {
        uiRenderer_.showMessage("Invalid coordinate.", MessageTone::Danger);
        return false;
    }

    if (!canDeployMineAt(target)) {
        uiRenderer_.showMessage("That location is out of range or already marked.", MessageTone::Warning);
        return false;
    }

    mines_.push_back(target);
    enemyBoard_.setCell(target, 'M');
    addLog("Mine deployed at " + toCoordinate(target) + ".");
    uiRenderer_.showMessage("Mine deployed successfully. This sector is now trapped.", MessageTone::Special);
    return true;
}

bool GameEngine::saveGame() {
    uiRenderer_.showActionScreen("SAVE", {"Saving your current game..."});
    uiRenderer_.showLoading("Saving", MessageTone::Special, 4, 100);
    if (saveManager_.saveToFile("savegame.txt", makeSaveData())) {
        addLog("Command deck saved to savegame.txt.");
        uiRenderer_.showMessage("Game saved.", MessageTone::Success);
        return true;
    }

    uiRenderer_.showMessage("Save failed.", MessageTone::Danger);
    return false;
}

void GameEngine::enemyPhase() {
    const int convoyHpBefore = convoy_.hp();
    const int destroyerHpBefore = destroyer_.hp();
    const int radarHpBefore = radarShip_.hp();

    advanceConvoy();
    enemyManager_.updateEnemies(destroyer_, radarShip_, convoy_, mines_, missionLog_, randomEngine_, weather_);

    enemyBoard_.clearSymbol('M');
    enemyBoard_.clearTransientSignals();
    refreshRadarVision();
    for (const Position& mine : mines_) {
        enemyBoard_.setCell(mine, 'M');
    }

    for (const Ship& enemy : enemyManager_.enemies()) {
        if (!enemy.isAlive()) {
            for (const Position& position : enemy.getOccupiedCells()) {
                enemyBoard_.markHit(position);
            }
        }
    }

    if (convoy_.hp() < convoyHpBefore) {
        uiRenderer_.showAlert("ALERT",
                              {"ENEMY TORPEDO INCOMING",
                               "The convoy has been hit!",
                               "Convoy HP reduced by " + std::to_string(convoyHpBefore - convoy_.hp()) + "."},
                              MessageTone::Danger);
    } else if (destroyer_.hp() < destroyerHpBefore) {
        uiRenderer_.showAlert("ALERT",
                              {"Enemy fire struck the destroyer.",
                               "Destroyer HP reduced by " + std::to_string(destroyerHpBefore - destroyer_.hp()) + "."},
                              MessageTone::Danger);
    } else if (radarShip_.hp() < radarHpBefore) {
        uiRenderer_.showAlert("ALERT",
                              {"Enemy fire struck the radar ship.",
                               "Radar Ship HP reduced by " + std::to_string(radarHpBefore - radarShip_.hp()) + "."},
                              MessageTone::Danger);
    }
}

void GameEngine::advanceConvoy() {
    if (!convoy_.isAlive() || convoyEscaped_) {
        return;
    }

    std::vector<Position> nextCells = convoy_.getOccupiedCells();
    bool escapesMap = false;
    for (Position& position : nextCells) {
        ++position.col;
        if (position.col >= 10) {
            escapesMap = true;
        }
    }

    if (escapesMap) {
        convoyEscaped_ = true;
        addLog("The convoy enters the safe zone.");
        return;
    }

    const std::vector<Position> blocked = occupiedFriendlyCells(true, true, false);
    if (!enemyBoard_.canPlace(nextCells, blocked)) {
        addLog("Convoy advance delayed by allied positioning.");
        return;
    }

    convoy_.setOccupiedCells(nextCells);
    addLog("Convoy advances to sector " + toCoordinate(convoy_.getOccupiedCells().back()) + ".");
}

void GameEngine::updateWeather() {
    std::uniform_int_distribution<int> weatherRoll(0, 2);
    const int value = weatherRoll(randomEngine_);
    if (value == 0) {
        weather_ = "Clear";
    } else if (value == 1) {
        weather_ = "Fog";
    } else {
        weather_ = "Storm";
    }

    addLog("Weather update: " + weather_ + ".");
    if (turnNumber_ > 1) {
        if (weather_ == "Fog") {
            uiRenderer_.showAlert("WEATHER",
                                  {"FOG IS FORMING OVER THE SEA",
                                   "Visibility reduced this turn."},
                                  MessageTone::Warning);
        } else if (weather_ == "Storm") {
            uiRenderer_.showAlert("STORM",
                                  {"Weapon accuracy reduced by severe weather.",
                                   "Radar interference expected this turn."},
                                  MessageTone::Special);
        } else {
            uiRenderer_.showAlert("WEATHER",
                                  {"Skies are clear. Detection and accuracy stabilizing."},
                                  MessageTone::Success);
        }
    }
}

void GameEngine::refreshRadarVision() {
    if (!radarShip_.isAlive()) {
        return;
    }

    const Position center = radarShip_.getOccupiedCells().front();
    const std::vector<Position> visibleArea = getArea(center, 1);

    for (const Ship& enemy : enemyManager_.enemies()) {
        if (!enemy.isAlive()) {
            continue;
        }

        const Position enemyPosition = enemy.getOccupiedCells().front();
        if (std::find(visibleArea.begin(), visibleArea.end(), enemyPosition) != visibleArea.end() &&
            enemyBoard_.getCell(enemyPosition) == '~') {
            enemyBoard_.setCell(enemyPosition, '?');
        }
    }
}

void GameEngine::addLog(const std::string& entry) {
    missionLog_.push_front(entry);
    while (missionLog_.size() > 5) {
        missionLog_.pop_back();
    }
}

void GameEngine::pauseForEnter() const {
    uiRenderer_.showMessage("Press Enter to continue...", MessageTone::Info);
    std::string ignored;
    std::getline(std::cin, ignored);
}

bool GameEngine::canUseTorpedoAt(const Position& target) const {
    const int destroyerRange = weather_ == "Fog" ? 4 : 5;
    const int radarRange = weather_ == "Fog" ? 2 : 3;

    if (destroyer_.isAlive() &&
        manhattanDistance(destroyer_.getOccupiedCells().front(), target) <= destroyerRange) {
        return true;
    }
    return radarShip_.isAlive() &&
           manhattanDistance(radarShip_.getOccupiedCells().front(), target) <= radarRange;
}

bool GameEngine::canScanAt(const Position& target) const {
    return enemyBoard_.isValidCoordinate(target);
}

bool GameEngine::canDeployMineAt(const Position& target) const {
    if (manhattanDistance(destroyer_.getOccupiedCells().front(), target) > 2) {
        return false;
    }
    return enemyBoard_.getCell(target) == '~';
}

std::vector<Ship> GameEngine::friendlyShips() const {
    return {destroyer_, radarShip_, convoy_};
}

std::vector<Position> GameEngine::occupiedFriendlyCells(bool includeDestroyer, bool includeRadar, bool includeConvoy) const {
    std::vector<Position> cells;

    auto addCells = [&](const Ship& ship) {
        if (!ship.isAlive()) {
            return;
        }
        const std::vector<Position>& occupied = ship.getOccupiedCells();
        cells.insert(cells.end(), occupied.begin(), occupied.end());
    };

    if (includeDestroyer) {
        addCells(destroyer_);
    }
    if (includeRadar) {
        addCells(radarShip_);
    }
    if (includeConvoy) {
        addCells(convoy_);
    }

    return cells;
}

SaveData GameEngine::makeSaveData() const {
    SaveData saveData;
    saveData.enemyBoard = enemyBoard_;
    saveData.destroyer = destroyer_;
    saveData.radarShip = radarShip_;
    saveData.convoy = convoy_;
    saveData.enemyManager = enemyManager_;
    saveData.mission = mission_;
    saveData.turnNumber = turnNumber_;
    saveData.score = score_;
    saveData.weather = weather_;
    saveData.mines = mines_;
    saveData.convoyEscaped = convoyEscaped_;
    saveData.missionLog = missionLog_;
    return saveData;
}

void GameEngine::applySaveData(const SaveData& saveData) {
    enemyBoard_ = saveData.enemyBoard;
    destroyer_ = saveData.destroyer;
    radarShip_ = saveData.radarShip;
    convoy_ = saveData.convoy;
    enemyManager_ = saveData.enemyManager;
    mission_ = saveData.mission;
    turnNumber_ = saveData.turnNumber;
    score_ = saveData.score;
    weather_ = saveData.weather;
    mines_ = saveData.mines;
    convoyEscaped_ = saveData.convoyEscaped;
    missionLog_ = saveData.missionLog;
}

std::string GameEngine::currentThreatLevel() const {
    int livingEnemies = 0;
    for (const Ship& enemy : enemyManager_.enemies()) {
        if (enemy.isAlive()) {
            ++livingEnemies;
        }
    }

    if (livingEnemies >= 3) {
        return "HIGH";
    }
    if (livingEnemies == 2) {
        return "MEDIUM";
    }
    if (livingEnemies == 1) {
        return "LOW";
    }
    return "CLEAR";
}

int GameEngine::enemiesDestroyed() const {
    int destroyed = 0;
    for (const Ship& enemy : enemyManager_.enemies()) {
        if (!enemy.isAlive()) {
            ++destroyed;
        }
    }
    return destroyed;
}
