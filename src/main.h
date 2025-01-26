#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <string>
#include "chess.hpp"
#include "evaluation.h"
#include "reader.hpp"
#include "ordering.h"
#include "transposition.h"
#include "timeman.h"
#include "killers.h"

// Function to get book move 
std::string getBookMove(const chess::Board& board);

// Returns the best move for the current position 
std::string getBestMove(std::string fen, int timeLeft, int increment, int movesToGo, int fixedMoveTime = 0);

// Quiescence search function
int quiescence(chess::Board& board, int alpha, int beta, TranspositionTable& tt, int ply);

// Negamax function for move evaluation (with alpha-beta pruning)
int negamax(chess::Board& board, int depth, int alpha, int beta, TranspositionTable& tt, int ply, TimeManager& tm, int numExtensions = 0, bool allowNull = true);

// To export: g++ -static -shared -o chess_engine.dll *.cpp

#endif 