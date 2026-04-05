#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "Board.h"
#include "EnemyManager.h"
#include "Mission.h"
#include "SaveManager.h"
#include "UIRenderer.h"

#include <deque>
#include <random>
#include <string>
#include <vector>

class GameEngine {
public:
    GameEngine();
    void run();

private:
    Board enemyBoard_;
    Ship destroyer_;
    Ship radarShip_;
    Ship convoy_;
    EnemyManager enemyManager_;
    Mission mission_;
    SaveManager saveManager_;
    UIRenderer uiRenderer_;
    std::mt19937 randomEngine_;
    std::deque<std::string> missionLog_;
    std::vector<Position> mines_;
    int turnNumber_;
    int score_;
    bool convoyEscaped_;
    std::string weather_;

    void startNewGame();
    bool loadGame();
    void playActiveMission();
    void processTurn();

    bool handleMoveShip();
    bool handleRadarScan();
    bool handleFireTorpedo();
    bool handleDeployMine();
    bool saveGame();

    void enemyPhase();
    void advanceConvoy();
    void updateWeather();
    void addLog(const std::string& entry);
    void pauseForEnter() const;
    std::string currentThreatLevel() const;
    int enemiesDestroyed() const;

    bool canUseTorpedoAt(const Position& target) const;
    bool canScanAt(const Position& target) const;
    bool canDeployMineAt(const Position& target) const;

    std::vector<Ship> friendlyShips() const;
    std::vector<Position> occupiedFriendlyCells(bool includeDestroyer, bool includeRadar, bool includeConvoy) const;

    SaveData makeSaveData() const;
    void applySaveData(const SaveData& saveData);
};

#endif
