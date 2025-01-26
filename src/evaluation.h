#ifndef EVALUATION_H
#define EVALUATION_H

#include "chess.hpp"

// Piece Values
enum PieceValue {
    PAWN = 100,
    KNIGHT = 320,
    BISHOP = 330,
    ROOK = 500,
    QUEEN = 900,
    KING = 20000
};

// Declare tables as extern
extern const int MG_PAWN_TABLE[64];
extern const int EG_PAWN_TABLE[64];
extern const int MG_KNIGHT_TABLE[64];
extern const int EG_KNIGHT_TABLE[64];
extern const int MG_BISHOP_TABLE[64];
extern const int EG_BISHOP_TABLE[64];
extern const int MG_ROOK_TABLE[64];
extern const int EG_ROOK_TABLE[64];
extern const int MG_QUEEN_TABLE[64];
extern const int EG_QUEEN_TABLE[64];
extern const int MG_KING_TABLE[64];
extern const int EG_KING_TABLE[64];
extern const int* MG_TABLE[6];
extern const int* EG_TABLE[6];
extern const int FLIP[64];

// Material Information 
struct MaterialInfo {
    chess::Bitboard whitePawns;
    chess::Bitboard blackPawns;
    int materialScore;
};

// Passed Pawn Evaluation 
extern chess::Bitboard passedPawnMask[2][64];
extern chess::Bitboard getPassedPawnMask(int square, chess::Color color);
extern void initPassedPawnMasks();

// Doubled Pawn Evaluation
extern const int DOUBLED_PAWN_PENALTY;
extern chess::Bitboard fileMasks[8];
extern void initExclusiveFileMasks();

// Isolated Pawn Evaluation
extern const int ISOLATED_PAWN_PENALTY;
extern chess::Bitboard leftAndRightFileMasks[8];
extern void initLeftAndRightFileMasks();

// Initialize masks
extern void initMasks();

// Declare evaluation functions as extern
extern MaterialInfo materialEval(const chess::Board& board);
extern int gamePhase(const chess::Board& board);
int pawnsEval(const chess::Bitboard& whitePawns, const chess::Bitboard& blackPawns);
extern int pstEval(const chess::Board& board, int mgPhase);
extern int evaluate(const chess::Board& board);

// Game phases (for tapered evaluation)
enum GamePhaseValues {
    P = 0,
    N = 1,
    B = 1,
    R = 2,
    Q = 4,
    K = 0
};

#endif