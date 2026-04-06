#ifndef UIRENDERER_H
#define UIRENDERER_H

#include "Board.h"
#include "Mission.h"

#include <deque>
#include <string>
#include <vector>

enum class MessageTone {
    Neutral,
    Info,
    Success,
    Warning,
    Danger,
    Special
};

class UIRenderer {
public:
    UIRenderer();

    void clearScreen() const;
    void showMainMenu() const;
    void showHelp() const;
    void showMissionBriefing(const Mission& mission,
                             const std::vector<Ship>& friendlyShips,
                             const std::string& threatLevel) const;
    void renderGame(const Mission& mission,
                    int turnNumber,
                    const std::string& weather,
                    int score,
                    const std::string& threatLevel,
                    const Board& enemyBoard,
                    const std::vector<Ship>& friendlyShips,
                    const std::deque<std::string>& missionLog) const;

    void showActionScreen(const std::string& title, const std::vector<std::string>& lines) const;
    void showAlert(const std::string& title, const std::vector<std::string>& lines, MessageTone tone) const;
    void showLoading(const std::string& label, MessageTone tone, int steps = 4, int delayMs = 140) const;
    void showEndScreen(bool victory,
                       const std::string& reason,
                       int score,
                       int turnNumber,
                       int enemiesDestroyed) const;
    void showMessage(const std::string& message, MessageTone tone = MessageTone::Neutral) const;

private:
    void printHeader(const Mission& mission,
                     int turnNumber,
                     const std::string& weather,
                     int score,
                     const std::string& threatLevel) const;
    void printBoardSection(const std::string& title, const std::vector<std::string>& lines) const;
    void printStatus(const std::vector<Ship>& friendlyShips, const std::string& weather) const;
    void printActions() const;
    void printEnemyIntel() const;
    void printLog(const std::deque<std::string>& missionLog) const;
    std::string hpBar(int current, int maximum, int width = 10) const;
    const char* healthColor(int current, int maximum) const;
    std::string styleBoardLine(const std::string& line) const;
    void printPanel(const std::string& title,
                    const std::vector<std::string>& lines,
                    MessageTone tone = MessageTone::Info) const;
    void printAnimatedTitle() const;

    mutable bool titleAnimated_;
};

#endif
