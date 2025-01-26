#ifndef ORDERING_H
#define ORDERING_H

#include "chess.hpp"
#include "transposition.h"
#include "killers.h"

int getPieceValue(chess::PieceType& piece);

void OrderMoves(chess::Board& board, chess::Movelist& moves, TranspositionTable& tt, uint64_t key, KillerMoves& killers, HistoryTable& history, int ply);

#endif