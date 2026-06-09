#include "Bot.h"
#include "GameLogic.h"
#include <climits>
#include <vector>
#include <cctype>

const int PAWN_TABLE[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    { 5,  5, 10, 25, 25, 10,  5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0},
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 5, 10, 10,-20,-20, 10, 10,  5},
    { 0,  0,  0,  0,  0,  0,  0,  0}
};

const int KNIGHT_TABLE[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

const int BISHOP_TABLE[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

const int ROOK_TABLE[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};

const int QUEEN_TABLE[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

const int KING_MIDDLE_TABLE[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20},
    { 20, 30, 10,  0,  0, 10, 30, 20}
};

int Bot::pieceValue(char piece) {
    switch (tolower(piece)) {
        case 'p': return 100;
        case 'n': return 320;
        case 'b': return 330;
        case 'r': return 500;
        case 'q': return 900;
        case 'k': return 20000;
        default:  return 0;
    }
}

int Bot::positionalBonus(char piece, int row, int col, Color botColor) {
    int r = (Board::pieceColor(piece) == Color::White) ? row : (7 - row);
    switch (tolower(piece)) {
        case 'p': return PAWN_TABLE[r][col];
        case 'n': return KNIGHT_TABLE[r][col];
        case 'b': return BISHOP_TABLE[r][col];
        case 'r': return ROOK_TABLE[r][col];
        case 'q': return QUEEN_TABLE[r][col];
        case 'k': return KING_MIDDLE_TABLE[r][col];
        default:  return 0;
    }
}

int Bot::evaluate(const Board& board, Color botColor) {
    int score = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            char piece = board.grid[r][c];
            if (piece == ' ') continue;
            int val = pieceValue(piece) + positionalBonus(piece, r, c, botColor);
            if (Board::pieceColor(piece) == botColor)
                score += val;
            else
                score -= val;
        }
    }
    return score;
}

int Bot::getEval(const Board& board, Color color) {
    if (GameLogic::isCheckmate(board, board.currentTurn)) {
        Color justMoved = (board.currentTurn == Color::White) ?
                          Color::Black : Color::White;
        return (justMoved == color) ? 100000 : -100000;
    }
    if (GameLogic::isStalemate(board, board.currentTurn)) return 0;
    return evaluate(board, color);
}

int Bot::minimax(Board board, int depth, bool isMaximizing,
                 Color botColor, int alpha, int beta)
{
    Color humanColor = (botColor == Color::White) ? Color::Black : Color::White;

    // Verifica mat/pat pentru culoarea care urmeaza sa mute
    if (GameLogic::isCheckmate(board, board.currentTurn)) {
        Color justMoved = (board.currentTurn == Color::White) ?
                          Color::Black : Color::White;
        if (justMoved == botColor) return  100000 + depth;
        else                       return -100000 - depth;
    }
    if (GameLogic::isStalemate(board, board.currentTurn)) return 0;
    if (depth == 0) return evaluate(board, botColor);

    Color currentColor = isMaximizing ? botColor : humanColor;

    if (isMaximizing)
    {
        int best = INT_MIN;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (Board::pieceColor(board.grid[r][c]) != currentColor) continue;
                auto moves = GameLogic::getLegalMoves(board, r, c);
                for (auto& move : moves) {
                    Board copy = board;
                    copy.applyMove(move);
                    int score = minimax(copy, depth - 1, false, botColor, alpha, beta);
                    best  = std::max(best, score);
                    alpha = std::max(alpha, best);
                    if (beta <= alpha) goto done_max;
                }
            }
        }
        done_max:
        return best == INT_MIN ? 0 : best;
    }
    else
    {
        int best = INT_MAX;
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                if (Board::pieceColor(board.grid[r][c]) != currentColor) continue;
                auto moves = GameLogic::getLegalMoves(board, r, c);
                for (auto& move : moves) {
                    Board copy = board;
                    copy.applyMove(move);
                    int score = minimax(copy, depth - 1, true, botColor, alpha, beta);
                    best = std::min(best, score);
                    beta = std::min(beta, best);
                    if (beta <= alpha) goto done_min;
                }
            }
        }
        done_min:
        return best == INT_MAX ? 0 : best;
    }
}

Move Bot::getBestMove(const Board& board, Color color, int depth) {
    int best  = INT_MIN;
    Move bestMove{0, 0, 0, 0};
    int alpha = INT_MIN;
    int beta  = INT_MAX;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (Board::pieceColor(board.grid[r][c]) != color) continue;
            auto moves = GameLogic::getLegalMoves(board, r, c);
            for (auto& move : moves) {
                Board copy = board;
                copy.applyMove(move);
                int score = minimax(copy, depth - 1, false, color, alpha, beta);
                if (score > best) {
                    best     = score;
                    bestMove = move;
                }
                alpha = std::max(alpha, best);
            }
        }
    }
    return bestMove;
}

int Bot::getFullEval(const Board& board, Color color, int depth) {
    if (GameLogic::isCheckmate(board, board.currentTurn)) {
        Color justMoved = (board.currentTurn == Color::White) ?
                          Color::Black : Color::White;
        return (justMoved == color) ? 100000 : -100000;
    }
    if (GameLogic::isStalemate(board, board.currentTurn)) return 0;

    // dupa ce `color` a mutat, urmeaza adversarul
    // deci next e minimizator pentru `color`
    bool nextIsMaximizing = (board.currentTurn == color);
    return minimax(board, depth, nextIsMaximizing, color, INT_MIN, INT_MAX);
}

int Bot::scoreMoveWithMinimax(const Board& board, const Move& move,
                               Color color, int depth)
{
    Board copy = board;
    copy.applyMove(move);
    bool nextIsMaximizing = (copy.currentTurn == color);
    if (GameLogic::isCheckmate(copy, copy.currentTurn)) {
        Color justMoved = (copy.currentTurn == Color::White) ?
                          Color::Black : Color::White;
        return (justMoved == color) ? 100000 : -100000;
    }
    if (GameLogic::isStalemate(copy, copy.currentTurn)) return 0;
    return minimax(copy, depth - 1, nextIsMaximizing, color, INT_MIN, INT_MAX);
}

std::pair<Move, int> Bot::getBestMoveWithScore(const Board& board,
                                                Color color, int depth)
{
    int best  = INT_MIN;
    Move bestMove{0, 0, 0, 0};
    int alpha = INT_MIN;
    int beta  = INT_MAX;

    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (Board::pieceColor(board.grid[r][c]) != color) continue;
            auto moves = GameLogic::getLegalMoves(board, r, c);
            for (auto& move : moves) {
                Board copy = board;
                copy.applyMove(move);
                bool nextIsMaximizing = (copy.currentTurn == color);
                if (GameLogic::isCheckmate(copy, copy.currentTurn)) {
                    Color justMoved = (copy.currentTurn == Color::White) ?
                                      Color::Black : Color::White;
                    int score = (justMoved == color) ? 100000 : -100000;
                    if (score > best) { best = score; bestMove = move; }
                    alpha = std::max(alpha, best);
                    continue;
                }
                if (GameLogic::isStalemate(copy, copy.currentTurn)) continue;
                int score = minimax(copy, depth - 1, nextIsMaximizing,
                                    color, alpha, beta);
                if (score > best) { best = score; bestMove = move; }
                alpha = std::max(alpha, best);
            }
        }
    }
    return {bestMove, best};
}