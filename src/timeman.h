#ifndef TIMEMAN_H 
#define TIMEMAN_H

#include <chrono>
#include <algorithm>
#include <cmath>

class TimeManager {
    private:
        std::chrono::steady_clock::time_point start;
        std::chrono::milliseconds optimumTime;
        std::chrono::milliseconds maximumTime;
        std::chrono::milliseconds moveTime;
        uint64_t nodes;
        bool stopped;
        bool useFixedTime = false;
    public:
        TimeManager() : stopped(false), nodes(0) {};
        void init(int timeLeft, int increment, int movesToGo, int timeForMove = 0);
        void incrementNodes() { nodes++; };
        bool shouldStop();
        void stop() { stopped = true; }
};

#endif