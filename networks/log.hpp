#pragma once

#include <iostream>
#include <string>

// console colours
#define YLW "\033[1;33m"
#define RED "\033[1;31m"
#define GRN "\033[1;32m"
#define RS "\033[1m"

enum State {
    Neutral, Error, Success, None
};

// logs the provided string to the console.
void log(const std::string msg, State state = State::Neutral) {
    std::string clr;
    switch (state) {
        case Neutral:
        clr = YLW;
        case Error:
        clr = RED;
        case Success:
        clr = GRN;
        case None:
        clr = "";
    }
    if (state == Error || state == Success) {
        std::cout << clr + msg + RS << std::endl;
    }
}

// log, but fatal.
void logFatal(const std::string msg, int stat = 1) {
    log(msg, State::Error);
    exit(stat);
}
