#include "killers.h"

void KillerMoves::addKillerMove(chess::Move move, int ply) {
    if (move==killers[ply][0]) return; 

    this->killers[ply][1] = killers[ply][0];
    this->killers[ply][0] = move;
}

bool KillerMoves::isKiller(chess::Move move, int ply) const {
    return move==killers[ply][0] || move==killers[ply][1];
}

void KillerMoves::clear() {
    for (int i=0; i<MAX_PLY; i++) {
        for (int j=0; j<MAX_KILLERS; j++) {
            this->killers[i][j] = chess::Move::NO_MOVE;
        }
    }
}

void HistoryTable::clear() {
    memset(this->table, 0, sizeof(this->table));
}

void HistoryTable::update(const chess::Board& board, const chess::Move& move, int depth) {
    const int bonus = depth * depth;
    const int colour = board.sideToMove();
    const int from = move.from().index();
    const int to = move.to().index();
    const int clampedBonus = std::clamp(bonus, -20000, 20000);
    this->table[colour][from][to] += clampedBonus - this->table[colour][from][to] * std::abs(clampedBonus) / MAX_HISTORY;
}

int HistoryTable::get(const chess::Board& board, const chess::Move& move) const {
    int color = board.sideToMove();
    return this->table[color][move.from().index()][move.to().index()];
}