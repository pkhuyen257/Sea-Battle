#include "UIRenderer.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

namespace {
const char* RESET = "\033[0m";
const char* BOLD = "\033[1m";
const char* RED = "\033[31m";
const char* GREEN = "\033[32m";
const char* YELLOW = "\033[33m";
const char* BLUE = "\033[34m";
const char* MAGENTA = "\033[35m";
const char* CYAN = "\033[36m";
const char* WHITE = "\033[37m";

const int kPanelWidth = 78;
const int kContentWidth = kPanelWidth - 4;
const int kTitleWidth = 104;

std::string colorize(const std::string& text, const char* color, bool bold = false) {
    return std::string(bold ? BOLD : "") + color + text + RESET;
}

const char* toneColor(MessageTone tone) {
    switch (tone) {
    case MessageTone::Info:
        return CYAN;
    case MessageTone::Success:
        return GREEN;
    case MessageTone::Warning:
        return YELLOW;
    case MessageTone::Danger:
        return RED;
    case MessageTone::Special:
        return MAGENTA;
    case MessageTone::Neutral:
    default:
        return WHITE;
    }
}

std::size_t visibleLength(const std::string& text) {
    std::size_t length = 0;
    bool inEscape = false;

    for (char ch : text) {
        if (!inEscape && ch == '\033') {
            inEscape = true;
            continue;
        }
        if (inEscape) {
            if (ch == 'm') {
                inEscape = false;
            }
            continue;
        }
        ++length;
    }

    return length;
}

std::string padVisible(const std::string& text, std::size_t width) {
    const std::size_t length = visibleLength(text);
    if (length >= width) {
        return text;
    }
    return text + std::string(width - length, ' ');
}

std::string centered(const std::string& text, std::size_t width) {
    const std::size_t length = visibleLength(text);
    if (length >= width) {
        return text;
    }

    const std::size_t left = (width - length) / 2;
    const std::size_t right = width - length - left;
    return std::string(left, ' ') + text + std::string(right, ' ');
}

std::string panelBorder(const std::string& title) {
    std::string label = " " + title + " ";
    if (label.size() > static_cast<std::size_t>(kPanelWidth - 2)) {
        label = label.substr(0, kPanelWidth - 2);
    }

    const int fill = (kPanelWidth - 2) - static_cast<int>(label.size());
    const int left = fill / 2;
    const int right = fill - left;
    return "+" + std::string(left, '=') + label + std::string(right, '=') + "+";
}

int consoleWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(console, &info)) {
        return info.srWindow.Right - info.srWindow.Left + 1;
    }
#endif
    return 120;
}

std::string leftPad(std::size_t width) {
    const int padding = std::max(0, (consoleWidth() - static_cast<int>(width)) / 2);
    return std::string(static_cast<std::size_t>(padding), ' ');
}

void sleepMs(int delayMs) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));
}
} // namespace

UIRenderer::UIRenderer() : titleAnimated_(false) {
#ifdef _WIN32
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (console != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(console, &mode)) {
            SetConsoleMode(console, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
    SetConsoleOutputCP(CP_UTF8);
#endif
}

void UIRenderer::clearScreen() const {
    std::cout << "\033[2J\033[H";
}

void UIRenderer::showMainMenu() const {
    clearScreen();
    printAnimatedTitle();
    const std::string pad = leftPad(kPanelWidth);
    std::cout << '\n';
    std::cout << pad << colorize(centered("NAVAL TACTICS TERMINAL", kPanelWidth), CYAN, false) << '\n';
    std::cout << '\n';
    std::cout << pad << colorize(centered("~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~", kPanelWidth), BLUE, false) << '\n';
    std::cout << '\n';
    std::cout << pad << colorize(centered("                 |\\", kPanelWidth), WHITE, false) << '\n';
    std::cout << pad << colorize(centered("          _______|_\\_______", kPanelWidth), WHITE, false) << '\n';
    std::cout << pad << colorize(centered("     _____/_______________/|_____", kPanelWidth), WHITE, false) << '\n';
    std::cout << pad << colorize(centered("     \\__________________________/", kPanelWidth), WHITE, false) << '\n';
    std::cout << pad << colorize(centered("        \\_/                \\_/", kPanelWidth), WHITE, false) << '\n';
    std::cout << '\n';
    std::cout << pad << colorize(centered("[1] New Game", kPanelWidth), GREEN, false) << '\n';
    std::cout << pad << colorize(centered("[2] Load Game", kPanelWidth), WHITE, false) << '\n';
    std::cout << pad << colorize(centered("[3] How To Play", kPanelWidth), WHITE, false) << '\n';
    std::cout << pad << colorize(centered("[4] Exit", kPanelWidth), WHITE, false) << '\n';
}

void UIRenderer::showHelp() const {
    clearScreen();
    std::vector<std::string> lines;
    lines.push_back("Goal: Keep the convoy alive until the mission ends.");
    lines.push_back("");
    lines.push_back("Each turn, choose one action:");
    lines.push_back("1. Move Ship");
    lines.push_back("2. Radar Scan");
    lines.push_back("3. Fire Torpedo");
    lines.push_back("4. Deploy Mine");
    lines.push_back("5. End Turn");
    lines.push_back("6. Save Game");
    lines.push_back("");
    lines.push_back("Simple flow:");
    lines.push_back("- Use Radar Ship to detect enemies.");
    lines.push_back("- Move Destroyer to intercept threats.");
    lines.push_back("- Protect the Convoy at all times.");
    lines.push_back("- End turn to let enemies and weather update.");
    lines.push_back("");
    lines.push_back("Enemy rules:");
    lines.push_back("- Enemies can only chase or attack ships they can see.");
    lines.push_back("- Each enemy does one thing: attack if in range,");
    lines.push_back("  otherwise move 1 tile toward a visible ship.");
    lines.push_back("- No diagonal movement.");
    lines.push_back("- If they see nothing, they patrol left.");
    lines.push_back("- Mines explode on contact.");
    lines.push_back("");
    lines.push_back("Enemy guide:");
    lines.push_back("- Patrol Boat: sight 3, range 2, damage 1");
    lines.push_back("- Raider: sight 4, range 3, damage 2");
    lines.push_back("- Submarine: sight 3, range 4, damage 2");
    lines.push_back("");
    lines.push_back("Symbols:");
    lines.push_back("~ Water   D Destroyer   R Radar Ship   C Convoy");
    lines.push_back("X Hit     o Miss        ? Signal       M Mine");
    printPanel("HOW TO PLAY", lines, MessageTone::Info);
}

void UIRenderer::showMissionBriefing(const Mission& mission,
                                     const std::vector<Ship>& friendlyShips,
                                     const std::string& threatLevel) const {
    clearScreen();

    std::vector<std::string> lines;
    lines.push_back(colorize(">>> INCOMING TRANSMISSION <<<", CYAN, true));
    lines.push_back("");
    lines.push_back("Operation: Safe Passage");
    lines.push_back("");
    lines.push_back("Enemy raiders and submarines have been detected nearby.");
    lines.push_back("Escort the convoy through hostile waters and survive");
    lines.push_back("until the mission timer is complete.");
    lines.push_back("");
    lines.push_back("Friendly Assets:");
    for (const Ship& ship : friendlyShips) {
        std::ostringstream assetLine;
        assetLine << "- " << ship.name() << " (HP " << ship.hp() << "/" << ship.maxHp() << ")";
        lines.push_back(assetLine.str());
    }
    lines.push_back("");
    lines.push_back("Objective: " + mission.objective());
    lines.push_back("Threat Level: " + colorize(
        threatLevel,
        toneColor(threatLevel == "HIGH" ? MessageTone::Danger
                                        : threatLevel == "MEDIUM" ? MessageTone::Warning
                                                                   : MessageTone::Success),
        true));
    lines.push_back("");
    lines.push_back("Enemy guide:");
    lines.push_back("- Patrol Boat: fast scout, light damage");
    lines.push_back("- Raider: main attacker, convoy threat");
    lines.push_back("- Submarine: harder to detect, long-range danger");
    printPanel("MISSION BRIEFING", lines, MessageTone::Info);
    std::cout << colorize("Press ENTER to deploy fleet...", GREEN, true) << '\n';
}

void UIRenderer::renderGame(const Mission& mission,
                            int turnNumber,
                            const std::string& weather,
                            int score,
                            const std::string& threatLevel,
                            const Board& enemyBoard,
                            const std::vector<Ship>& friendlyShips,
                            const std::deque<std::string>& missionLog) const {
    clearScreen();
    printHeader(mission, turnNumber, weather, score, threatLevel);
    printBoardSection("ENEMY WATERS", enemyBoard.renderEnemyBoard());

    Board helper;
    printBoardSection("YOUR FLEET", helper.renderFriendlyBoard(friendlyShips));
    printStatus(friendlyShips);
    printActions();
    printEnemyIntel();
    printLog(missionLog);
}

void UIRenderer::showActionScreen(const std::string& title, const std::vector<std::string>& lines) const {
    clearScreen();
    printPanel(title, lines, MessageTone::Info);
}

void UIRenderer::showAlert(const std::string& title, const std::vector<std::string>& lines, MessageTone tone) const {
    clearScreen();
    printPanel(title, lines, tone);
}

void UIRenderer::showLoading(const std::string& label, MessageTone tone, int steps, int delayMs) const {
    const char* color = toneColor(tone);
    for (int step = 1; step <= steps; ++step) {
        const int filled = (step * 10) / steps;
        const std::string bar = "[" + std::string(filled, '#') + std::string(10 - filled, '.') + "]";
        std::cout << colorize(label + " " + bar, color, true) << '\n';
        sleepMs(delayMs);
    }
}

void UIRenderer::showEndScreen(bool victory,
                               const std::string& reason,
                               int score,
                               int turnNumber,
                               int enemiesDestroyed) const {
    clearScreen();

    std::vector<std::string> lines;
    lines.push_back(reason);
    lines.push_back("");
    lines.push_back("Enemies destroyed: " + std::to_string(enemiesDestroyed));
    lines.push_back("Turns survived: " + std::to_string(turnNumber));
    lines.push_back("Final score: " + colorize(std::to_string(score), YELLOW, true));
    lines.push_back("");

    if (victory) {
        lines.push_back(centered("\\o/   \\o/   \\o/", kContentWidth));
        lines.push_back(centered(" |     |     | ", kContentWidth));
        lines.push_back(centered("/ \\   / \\   / \\", kContentWidth));
        printPanel("MISSION COMPLETE", lines, MessageTone::Success);
    } else {
        lines.push_back(centered(". . .", kContentWidth));
        lines.push_back(centered("___|_|___", kContentWidth));
        lines.push_back(centered("/         \\", kContentWidth));
        lines.push_back(centered("/___________\\", kContentWidth));
        lines.push_back(centered("/  |  \\", kContentWidth));
        printPanel("MISSION FAILED", lines, MessageTone::Danger);
    }
}

void UIRenderer::showMessage(const std::string& message, MessageTone tone) const {
    std::cout << leftPad(visibleLength(message))
              << colorize(message, toneColor(tone), tone != MessageTone::Neutral) << '\n';
}

void UIRenderer::printHeader(const Mission& mission,
                             int turnNumber,
                             const std::string& weather,
                             int score,
                             const std::string& threatLevel) const {
    std::vector<std::string> lines;
    lines.push_back(colorize(centered("SEA BATTLE: COMMAND DECK", kContentWidth), CYAN, true));
    lines.push_back(colorize(centered("TACTICAL ESCORT OPERATION", kContentWidth), WHITE, false));
    lines.push_back("Mission: " + mission.name());

    std::ostringstream turnLine;
    turnLine << "Turn: " << turnNumber << "/" << mission.maxTurns()
             << "    Weather: " << weather
             << "    Score: " << score;
    lines.push_back(turnLine.str());
    lines.push_back("Objective: " + mission.objective());
    lines.push_back("Threat Level: " + colorize(
        threatLevel,
        toneColor(threatLevel == "HIGH" ? MessageTone::Danger
                                        : threatLevel == "MEDIUM" ? MessageTone::Warning
                                                                   : MessageTone::Success),
        true));
    printPanel("GAME STATUS", lines, MessageTone::Info);
}

void UIRenderer::printBoardSection(const std::string& title, const std::vector<std::string>& lines) const {
    std::vector<std::string> styledLines;
    for (const std::string& line : lines) {
        styledLines.push_back(styleBoardLine(line));
    }
    printPanel(title, styledLines, MessageTone::Info);
}

void UIRenderer::printStatus(const std::vector<Ship>& friendlyShips) const {
    std::vector<std::string> lines;
    for (const Ship& ship : friendlyShips) {
        std::ostringstream line;
        line << std::left << std::setw(12) << ship.name()
             << " HP: " << ship.hp() << "/" << ship.maxHp() << "  "
             << hpBar(ship.hp(), ship.maxHp());
        lines.push_back(colorize(
            line.str(),
            toneColor(ship.hp() <= ship.maxHp() / 3 ? MessageTone::Danger
                                                    : ship.hp() <= (ship.maxHp() * 2) / 3 ? MessageTone::Warning
                                                                                           : MessageTone::Success),
            false));
    }
    printPanel("FLEET STATUS", lines, MessageTone::Info);
}

void UIRenderer::printActions() const {
    std::vector<std::string> lines;
    lines.push_back("[1] Move Ship   [2] Radar Scan   [3] Fire Torpedo");
    lines.push_back("[4] Deploy Mine [5] End Turn     [6] Save Game");
    printPanel("COMMAND OPTIONS", lines, MessageTone::Info);
}

void UIRenderer::printEnemyIntel() const {
    std::vector<std::string> lines;
    lines.push_back("Enemies react only to ships they can currently see.");
    lines.push_back("If they see nothing, they patrol left.");
    lines.push_back("Movement: 1 tile, row first, no diagonal.");
    lines.push_back("Patrol Boat  Sight 3  Range 2  Dmg 1");
    lines.push_back("Raider       Sight 4  Range 3  Dmg 2");
    lines.push_back("Submarine    Sight 3  Range 4  Dmg 2");
    lines.push_back("Tip: keep Destroyer close to Convoy.");
    printPanel("THREAT ANALYSIS", lines, MessageTone::Warning);
}

void UIRenderer::printLog(const std::deque<std::string>& missionLog) const {
    std::vector<std::string> lines;
    if (missionLog.empty()) {
        lines.push_back("- No recent activity.");
    } else {
        for (const std::string& entry : missionLog) {
            lines.push_back("- " + entry);
        }
    }
    printPanel("MISSION LOG", lines, MessageTone::Info);
}

std::string UIRenderer::hpBar(int current, int maximum, int width) const {
    if (maximum <= 0) {
        return "[----------]";
    }

    const int filled = static_cast<int>((static_cast<double>(current) / maximum) * width + 0.5);
    return "[" + colorize(std::string(filled, '#'), healthColor(current, maximum), true) +
           std::string(width - filled, '-') + "]";
}

const char* UIRenderer::healthColor(int current, int maximum) const {
    if (current <= maximum / 3) {
        return RED;
    }
    if (current <= (maximum * 2) / 3) {
        return YELLOW;
    }
    return GREEN;
}

std::string UIRenderer::styleBoardLine(const std::string& line) const {
    std::string styled;
    for (char ch : line) {
        if (ch == '~') {
            styled += colorize(std::string(1, ch), BLUE, false);
        } else if (ch == 'D' || ch == 'R' || ch == 'C') {
            styled += colorize(std::string(1, ch), GREEN, true);
        } else if (ch == 'X') {
            styled += colorize(std::string(1, ch), RED, true);
        } else if (ch == 'o') {
            styled += colorize(std::string(1, ch), WHITE, false);
        } else if (ch == '?') {
            styled += colorize(std::string(1, ch), YELLOW, true);
        } else if (ch == 'M') {
            styled += colorize(std::string(1, ch), MAGENTA, true);
        } else if ((ch >= 'A' && ch <= 'J') || (ch >= '0' && ch <= '9')) {
            styled += colorize(std::string(1, ch), WHITE, true);
        } else {
            styled += ch;
        }
    }
    return styled;
}

void UIRenderer::printPanel(const std::string& title,
                            const std::vector<std::string>& lines,
                            MessageTone tone) const {
    const char* color = toneColor(tone);
    const std::string pad = leftPad(kPanelWidth);
    std::cout << pad << colorize(panelBorder(title), color, true) << '\n';
    for (const std::string& line : lines) {
        std::cout << pad
                  << colorize("| ", color, true)
                  << padVisible(line, kContentWidth)
                  << colorize(" |", color, true) << '\n';
    }
    std::cout << pad << colorize("+" + std::string(kPanelWidth - 2, '=') + "+", color, true) << '\n';
}

void UIRenderer::printAnimatedTitle() const {
    const std::vector<std::string> titleArt = {
        " ███████╗███████╗ █████╗     ██████╗  █████╗ ████████╗████████╗██╗     ███████╗",
        " ██╔════╝██╔════╝██╔══██╗    ██╔══██╗██╔══██╗╚══██╔══╝╚══██╔══╝██║     ██╔════╝",
        " ███████╗█████╗  ███████║    ██████╔╝███████║   ██║      ██║   ██║     █████╗  ",
        " ╚════██║██╔══╝  ██╔══██║    ██╔══██╗██╔══██║   ██║      ██║   ██║     ██╔══╝  ",
        " ███████║███████╗██║  ██║    ██████╔╝██║  ██║   ██║      ██║   ███████╗███████╗",
        " ╚══════╝╚══════╝╚═╝  ╚═╝    ╚═════╝ ╚═╝  ╚═╝   ╚═╝      ╚═╝   ╚══════╝╚══════╝"
    };

    if (!titleAnimated_) {
        const std::string pad = leftPad(kTitleWidth);
        for (const std::string& line : titleArt) {
            std::cout << pad << colorize(centered(line, kTitleWidth), CYAN, true) << '\n';
            sleepMs(70);
        }
        std::cout << pad << colorize(centered("COMMAND DECK", kTitleWidth), WHITE, true) << '\n';
        std::cout << '\n';
        titleAnimated_ = true;
        return;
    }

    const std::string pad = leftPad(kTitleWidth);
    for (const std::string& line : titleArt) {
        std::cout << pad << colorize(centered(line, kTitleWidth), CYAN, true) << '\n';
    }
    std::cout << pad << colorize(centered("COMMAND DECK", kTitleWidth), WHITE, true) << '\n';
    std::cout << '\n';
}
