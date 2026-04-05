#include "Mission.h"

#include <istream>
#include <limits>
#include <ostream>
#include <utility>

Mission::Mission() : maxTurns_(8) {
}

Mission::Mission(std::string name, std::string objective, int maxTurns)
    : name_(std::move(name)), objective_(std::move(objective)), maxTurns_(maxTurns) {
}

const std::string& Mission::name() const {
    return name_;
}

const std::string& Mission::objective() const {
    return objective_;
}

int Mission::maxTurns() const {
    return maxTurns_;
}

bool Mission::isComplete(bool convoyEscaped) const {
    return convoyEscaped;
}

bool Mission::isFailed(int turnNumber, bool convoyEscaped, const Ship& convoy, const Ship& destroyer, const Ship& radarShip) const {
    if (!convoy.isAlive()) {
        return true;
    }
    if (!destroyer.isAlive() && !radarShip.isAlive()) {
        return true;
    }
    return turnNumber > maxTurns_ && !convoyEscaped;
}

void Mission::serialize(std::ostream& output) const {
    output << name_ << '\n';
    output << objective_ << '\n';
    output << maxTurns_ << '\n';
}

Mission Mission::deserialize(std::istream& input) {
    std::string name;
    std::string objective;
    int maxTurns = 8;
    std::getline(input, name);
    std::getline(input, objective);
    input >> maxTurns;
    input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return Mission(name, objective, maxTurns);
}
