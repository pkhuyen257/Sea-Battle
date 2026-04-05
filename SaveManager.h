#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "Board.h"
#include "EnemyManager.h"
#include "Mission.h"

#include <deque>
#include <string>
#include <vector>

struct SaveData {
    Board enemyBoard;
    Ship destroyer;
    Ship radarShip;
    Ship convoy;
    EnemyManager enemyManager;
    Mission mission;
    int turnNumber = 1;
    int score = 0;
    std::string weather = "Clear";
    std::vector<Position> mines;
    bool convoyEscaped = false;
    std::deque<std::string> missionLog;
};

class SaveManager {
public:
    bool saveToFile(const std::string& path, const SaveData& saveData) const;
    bool loadFromFile(const std::string& path, SaveData& saveData) const;
};

#endif
