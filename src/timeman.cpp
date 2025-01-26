#include "timeman.h"

void TimeManager::init(int timeLeft, int increment, int movesToGo, int timeForMove) {
    start = std::chrono::steady_clock::now();
    nodes = 0;
    stopped = false;

    if (timeForMove > 0) {
        useFixedTime = true;
        moveTime = std::chrono::milliseconds(timeForMove);
        optimumTime = moveTime;
        maximumTime = moveTime;
        return;
    }

    useFixedTime = false;

    auto baseTime = std::chrono::milliseconds(timeLeft);
    auto inc = std::chrono::milliseconds(increment);

    int mtg = movesToGo > 0 ? std::min(movesToGo, 40) : 25;

    const auto moveOverhead = std::chrono::milliseconds(200);

    auto timeLeftMs = std::max(std::chrono::milliseconds(1), baseTime + inc * (mtg - 1) - moveOverhead * (2 + mtg));

    double optScale;
    if (movesToGo == 0) {
        double remainingMoves = std::max(40.0, 50.0 - timeLeft/1000.0); // Estimate remaining moves
        optScale = 0.8 / remainingMoves;
        optScale = std::min(optScale, 0.06);
    } else {
        optScale = std::min(0.6 / movesToGo, 0.12);
    }

    optimumTime = std::chrono::milliseconds(static_cast<int>(optScale * timeLeftMs.count()));
    
    maximumTime = std::min(
        std::chrono::duration_cast<std::chrono::milliseconds>(optimumTime * 1.5), 
        std::chrono::milliseconds(static_cast<int>(0.4 * timeLeftMs.count()))
    );

    // Add extra for increment 
    if (increment > 0) {
        optimumTime += inc / 4;
        maximumTime += inc / 3;
    }
}

bool TimeManager::shouldStop() {
    if (stopped) {
        return true;
    }

    if ((nodes & 255) == 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);

        if (useFixedTime) {
            if (elapsed >= moveTime) {
                stopped = true;
                return true;
            }
        } else {
            if (elapsed >= optimumTime * 1.5 || elapsed >= maximumTime) {
                stopped = true;
                return true;
            }
        }
    }

    return false;
}