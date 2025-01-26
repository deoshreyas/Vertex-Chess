#include "main.h"

// 64MB transposition table
TranspositionTable tt(64);

// Killer Moves
KillerMoves killers;

// History Table
HistoryTable history;

const int MAX_EXTENSIONS = 16;

std::string getBookMove(const char *path, std::string fen) {
    Reader::Book openingBook;
    openingBook.Load(path);

    chess::Board board = chess::Board(fen);

    // Get zobrist hash of current position 
    uint64_t key = board.hash();

    Reader::BookMoves bookMoves = openingBook.GetBookMoves(key);

    if (!bookMoves.empty()) {
        Reader::BookMove bookMove = Reader::RandomBookMove(bookMoves);
        return Reader::ConvertBookMoveToUci(bookMove);
    }

    openingBook.Clear();

    return "";
}

int quiescence(chess::Board& board, int alpha, int beta, TranspositionTable& tt, int ply) {
    int stand_pat = evaluate(board);
    if (stand_pat >= beta) {
        return beta;
    }

    alpha = std::max(alpha, stand_pat);

    chess::Movelist captureMoves;
    chess::movegen::legalmoves<chess::movegen::MoveGenType::CAPTURE>(captureMoves, board);
    OrderMoves(board, captureMoves, tt, board.hash(), killers, history, ply);

    for (const auto& move : captureMoves) {
        board.makeMove(move);
        int score = -quiescence(board, -beta, -alpha, tt, ply + 1);
        board.unmakeMove(move);

        if (score >= beta) {
            return beta;
        }
        alpha = std::max(alpha, score);
    }

    return alpha;
}

int negamax(chess::Board& board, int depth, int alpha, int beta, TranspositionTable& tt, int ply, TimeManager& tm, int numExtensions, bool allowNull) {
    tm.incrementNodes();

    if (tm.shouldStop()) {
        return alpha;
    }

    if (ply > 0) {
        if (board.halfMoveClock() >= 100 || board.isRepetition(1)) {
            return 0; // Draw by 50-move rule or repetition
        }
    }
    
    if (depth==0) {
        return quiescence(board, alpha, beta, tt, ply);
    }

    uint64_t key = board.hash();

    // Probe the transposition table 
    TTEntry* entry = tt.probe(key);
    if (entry && entry->depth >= depth) {
        // Unadjust mate scores 
        int score = TTEntry::unadjustMateScore(entry->score, ply);

        if (entry->flag == TTEntry::EXACT) {
            return score;
        }
        if (entry->flag == TTEntry::ALPHA && score <= alpha) {
            return alpha;
        }
        if (entry->flag == TTEntry::BETA && score >= beta) {
            return beta;
        }
    }

    if (allowNull && depth >= 3 && !board.inCheck() && board.hasNonPawnMaterial(board.sideToMove())) {
        board.makeNullMove();
        int nullScore = -negamax(board, depth-3, -beta, -beta+1, tt, ply+1, tm, numExtensions, false);
        board.unmakeNullMove();
        if (nullScore >= beta && std::abs(nullScore) < INT_MAX) {
            return beta;
        }
    }

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);
    OrderMoves(board, moves, tt, key, killers, history, ply);

    if (moves.size() == 0) {
        if (board.inCheck()) {
            return -INT_MAX + ply; // Checkmate
        }
        return 0; // Stalemate
    }

    chess::Move bestMove;
    int originalAlpha = alpha;

    int movesSearched = 0;
    for (const auto& move : moves) {
        board.makeMove(move);       

        // Check extension 
        int extension = 0;
        if (numExtensions < MAX_EXTENSIONS && board.inCheck()) {
            extension = 1;
        }

        // Late move reduction
        bool needs_full_search = true;
        int score = 0;
        if (movesSearched>3 && depth>2 && !board.isCapture(move) && !extension) {
            int reduction = 1;
            score = -negamax(board, depth-1-reduction, -alpha-1, -alpha, tt, ply + 1, tm, numExtensions);
            needs_full_search = score > alpha;
        }
        if (needs_full_search) {
            score = -negamax(board, depth-1+extension, -beta, -alpha, tt, ply + 1, tm, numExtensions+extension);
        }
        board.unmakeMove(move);
        movesSearched++;

        if (score >= beta) {
            if (!board.isCapture(move)) {
                killers.addKillerMove(move, ply);
                history.update(board, move, depth);
            }
            tt.store(key, beta, depth, TTEntry::BETA, move, ply);
            return beta;
        }
        if (score > alpha) {
            alpha = score;
            bestMove = move;
        }
    }

    // Store position in transposition table 
    TTEntry::Flag flag;
    if (alpha <= originalAlpha) {
        flag = TTEntry::ALPHA;
    } else {
        flag = TTEntry::EXACT;
    }
    tt.store(key, alpha, depth, flag, bestMove, ply);

    return alpha;
}

std::string getBestMove(std::string fen, int timeLeft, int increment, int movesToGo, int fixedMoveTime) {
    initMasks();
    
    chess::Board board = chess::Board(fen);

    tt.incrementAge(); // Increment age at the start of each new search

    killers.clear(); // Reset killer moves at the start of each new search
    history.clear(); // Reset history table at the start of each new search

    // Time manager
    TimeManager tm;
    tm.init(timeLeft, increment, movesToGo, fixedMoveTime);

    chess::Move bestMove = chess::Move::NO_MOVE;
    int depth = 1;

    while (!tm.shouldStop()) {
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        OrderMoves(board, moves, tt, board.hash(), killers, history, 0);

        int alpha = -INT_MAX;
        int beta = INT_MAX;

        if (bestMove != chess::Move::NO_MOVE) {
            for (auto it = moves.begin(); it != moves.end(); ++it) {
                if (*it == bestMove) {
                    std::iter_swap(moves.begin(), it);
                    break;
                }
            }
        }

        chess::Move currentBestMove = moves.front();
        bool searchValid = true;

        for (const auto& move : moves) {
            if (tm.shouldStop()) {
                searchValid = false;
                break;
            }

            board.makeMove(move);
            int score = -negamax(board, depth-1, -beta, -alpha, tt, 1, tm);
            board.unmakeMove(move);

            if (tm.shouldStop()) {
                searchValid = false;
                break;
            }

            if (score > alpha) {
                alpha = score;
                currentBestMove = move;
            }
        }

        if (searchValid) {
            bestMove = currentBestMove;
            depth++;
        } else {
            break;
        }
    }
    
    return chess::uci::moveToUci(bestMove);
}

extern "C" {
    const char* getBestMove(const char* fen, int timeLeft, int increment, int movesToGo, int fixedMoveTime) {
        static std::string result;
        result = getBestMove(std::string(fen), timeLeft, increment, movesToGo, fixedMoveTime).c_str();
        return result.c_str();
    }

    const char* getBookMove(const char* path, const char* fen) {
        static std::string result;
        result = getBookMove(path, std::string(fen)).c_str();
        return result.c_str();
    }
}