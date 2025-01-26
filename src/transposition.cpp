#include "transposition.h"

TranspositionTable::TranspositionTable(size_t mb_size) {
    size = mb_size * 1024 * 1024 / sizeof(TTEntry);
    table.resize(size);
    currentAge = 0;
    clear();
}

void TranspositionTable::clear() {
    std::fill(table.begin(), table.end(), TTEntry());
}

void TranspositionTable::store(uint64_t key, int16_t score, uint8_t depth, TTEntry::Flag flag, const chess::Move bestMove, int ply) {
    size_t index = key % size;
    TTEntry* entry = &table[index];

    // Adjust mate score before storing 
    score = TTEntry::adjustMateScore(score, ply);

    // Replacement strategy:
    // 1. Always replace if empty slot (NONE flag)
    // 2. Always replace if current entry is from old search (different age)
    // 3. Always replace if new position has greater or equal depth 
    // 4. Keep existing entry if new position has lower depth
    if (entry->flag == TTEntry::NONE || entry->age != currentAge || depth >= entry->depth) {
        entry ->key = key;
        entry->score = score;
        entry->depth = depth;
        entry->flag = flag;
        entry->bestMove = bestMove;
        entry->age = currentAge;
    }
}

TTEntry* TranspositionTable::probe(uint64_t key) {
    size_t index = key % size;
    TTEntry* entry = &table[index];

    if (entry->key == key) {
        return entry;
    }

    return nullptr;
}