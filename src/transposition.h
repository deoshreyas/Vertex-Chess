#ifndef TRANSPOSITION_H 
#define TRANSPOSITION_H

#include <cstdint>
#include <vector>
#include "chess.hpp"

struct TTEntry {
    uint64_t key;          // Zobrist key
    int16_t score;         // Evaluation score
    uint8_t depth;         // Search depth
    uint8_t flag;          // Type of node (EXACT, ALPHA, BETA)
    chess::Move bestMove;  // Best move 
    uint8_t age;           // Age of entry

    // Helper functions for mate scores
    static int16_t adjustMateScore(int score, int ply) {
        if (score >= INT_MAX - 1000) {
            return score - ply; // Winning 
        }
        if (score <= -INT_MAX + 1000) {
            return score + ply; // Losing
        }
        return score;
    }

    static int16_t unadjustMateScore(int score, int ply) {
        if (score >= INT_MAX - 1000) {
            return score + ply; // Winning
        }
        if (score <= -INT_MAX + 1000) {
            return score - ply; // Losing
        }
        return score;
    }

    enum Flag : uint8_t {
        NONE = 0,          
        EXACT = 1,         // Exact score
        ALPHA = 2,         // Upper bound
        BETA = 3           // Lower bound
    };
};

class TranspositionTable {
    private:
        std::vector<TTEntry> table;
        size_t size;
        uint8_t currentAge;
    
    public:
        explicit TranspositionTable(size_t mb_size);
        void clear();
        void incrementAge() { currentAge++; }
        void store(uint64_t key, int16_t score, uint8_t depth, TTEntry::Flag flag, const chess::Move bestMove, int ply);
        TTEntry* probe(uint64_t);
};

#endif