#pragma once
#include <vector>
#include "Board.h"
#include "Move.h"

class GameLogic {
public:
    static bool isInCheck(const Board& board, Color color);
    static std::vector<Move> getLegalMoves(const Board& board, int row, int col);
    static bool isCheckmate(const Board& board, Color color);
    static bool isStalemate(const Board& board, Color color);
};