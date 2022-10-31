#pragma once

#include <iostream>
#include <string>
#include <map>
#include "../ext/pretty.hpp"

enum class State {
    Neutral, Error, Success
};

// logs the provided string to the console.
void log(const std::string msg, State state = State::Neutral) {
    switch (state) {
        case State::Neutral:
            std::cout << pty::paint(msg, "yellow") << std::endl;
            break;
        case State::Error:
            std::cout << pty::paint(msg, {"red", "bold"}) << std::endl;
            break;
        case State::Success:
            std::cout << pty::paint(msg, {"green", "bold"}) << std::endl;
            break;
    }
}

// log, but fatal.
void logFatal(const std::string msg, int stat = 1) {
    log(msg, State::Error);
    exit(stat);
}
