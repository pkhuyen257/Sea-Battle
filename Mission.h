#ifndef MISSION_H
#define MISSION_H

#include "Ship.h"

#include <iosfwd>
#include <string>

class Mission {
public:
    Mission();
    Mission(std::string name, std::string objective, int maxTurns);

    const std::string& name() const;
    const std::string& objective() const;
    int maxTurns() const;

    bool isComplete(bool convoyEscaped) const;
    bool isFailed(int turnNumber, bool convoyEscaped, const Ship& convoy, const Ship& destroyer, const Ship& radarShip) const;

    void serialize(std::ostream& output) const;
    static Mission deserialize(std::istream& input);

private:
    std::string name_;
    std::string objective_;
    int maxTurns_;
};

#endif
