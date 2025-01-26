#include "ordering.h"

int getPieceValue(chess::PieceType& piece) {
    int piece_underlying = static_cast<int>(piece);
    switch (piece_underlying) {
        case int(chess::PieceType::PAWN):
            return 100;
        case int(chess::PieceType::KNIGHT):
            return 320;
        case int(chess::PieceType::BISHOP):
            return 330;
        case int(chess::PieceType::ROOK):
            return 500;
        case int(chess::PieceType::QUEEN):
            return 900;
        default:
            return 0;
    }
}

void OrderMoves(chess::Board& board, chess::Movelist& moves, TranspositionTable& tt, uint64_t key, KillerMoves& killers, HistoryTable& history, int ply) {
    TTEntry* ttEntry = tt.probe(key);
    chess::Move ttMove = ttEntry ? ttEntry->bestMove : chess::Move::NO_MOVE;

    for (auto& move: moves) {
        int score = 0;
        
        // TT move gets highest priority
        if (ttMove != chess::Move::NO_MOVE && move == ttMove) {
            score = 20000;
        }

        // MVV - LVA (Most Valuable Victim - Least Valuable Aggressor) Heuristic
        else if (board.isCapture(move)) {
            chess::PieceType aggressor = board.at<chess::PieceType>(move.from());
            chess::PieceType victim = board.at<chess::PieceType>(move.to());
            score = 10000 + (10 * getPieceValue(victim) - getPieceValue(aggressor));
        }

        // Killer Moves 
        else if (killers.isKiller(move, ply)) {
            score = 9000;
        }

        // History Heuristic
        else {
            score = 5000 + history.get(board, move);
        }

        move.setScore(score);
    }

    std::sort(moves.begin(), moves.end(), [](const chess::Move& a, const chess::Move& b) {
        return a.score() > b.score();
    });
}