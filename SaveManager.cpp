#include "SaveManager.h"

#include <fstream>
#include <limits>

bool SaveManager::saveToFile(const std::string& path, const SaveData& saveData) const {
    std::ofstream output(path.c_str());
    if (!output) {
        return false;
    }

    output << "SEA_BATTLE_SAVE_V1\n";
    output << saveData.turnNumber << ' ' << saveData.score << ' ' << saveData.convoyEscaped << '\n';
    output << saveData.weather << '\n';
    saveData.mission.serialize(output);
    saveData.enemyBoard.serialize(output);
    saveData.destroyer.serialize(output);
    saveData.radarShip.serialize(output);
    saveData.convoy.serialize(output);
    saveData.enemyManager.serialize(output);

    output << saveData.mines.size() << '\n';
    for (const Position& mine : saveData.mines) {
        output << mine.row << ' ' << mine.col << '\n';
    }

    output << saveData.missionLog.size() << '\n';
    for (const std::string& line : saveData.missionLog) {
        output << line << '\n';
    }
    return true;
}

bool SaveManager::loadFromFile(const std::string& path, SaveData& saveData) const {
    std::ifstream input(path.c_str());
    if (!input) {
        return false;
    }

    std::string header;
    std::getline(input, header);
    if (header != "SEA_BATTLE_SAVE_V1") {
        return false;
    }

    input >> saveData.turnNumber >> saveData.score >> saveData.convoyEscaped;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(input, saveData.weather);
    saveData.mission = Mission::deserialize(input);
    saveData.enemyBoard = Board::deserialize(input);
    saveData.destroyer = Ship::deserialize(input);
    saveData.radarShip = Ship::deserialize(input);
    saveData.convoy = Ship::deserialize(input);
    saveData.enemyManager = EnemyManager::deserialize(input);

    std::size_t mineCount = 0;
    input >> mineCount;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    saveData.mines.clear();
    for (std::size_t index = 0; index < mineCount; ++index) {
        Position mine;
        input >> mine.row >> mine.col;
        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        saveData.mines.push_back(mine);
    }

    std::size_t logCount = 0;
    input >> logCount;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    saveData.missionLog.clear();
    for (std::size_t index = 0; index < logCount; ++index) {
        std::string line;
        std::getline(input, line);
        saveData.missionLog.push_back(line);
    }

    return true;
}
