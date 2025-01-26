#ifndef KILLERS_H 
#define KILLERS_H

#include "chess.hpp"
#include <cstring>

class KillerMoves {
    private:
        static constexpr int MAX_KILLERS = 2; 
        static constexpr int MAX_PLY = 100;
        chess::Move killers[MAX_PLY][MAX_KILLERS];

    public:
        void addKillerMove(chess::Move move, int ply);
        bool isKiller(chess::Move move, int ply) const;
        void clear();
};

struct HistoryTable {
    int table[2][64][64];
    static constexpr int MAX_HISTORY = 20000;

    void clear();
    void update(const chess::Board& board, const chess::Move& move, int depth);
    int get(const chess::Board& board, const chess::Move& move) const;
};

#endif