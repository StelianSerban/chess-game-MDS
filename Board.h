#pragma once
#include <vector>
#include "Move.h"

enum class Color { White, Black, None };

class Board {
public:
    char grid[8][8];
    Color currentTurn;

    Board();
    void reset();

    bool applyMove(const Move& move);
    void undoMove(const Move& move, char capturedPiece);

    static Color pieceColor(char piece);
    static bool isWhitePiece(char piece);
    static bool isBlackPiece(char piece);
    static bool inBounds(int row, int col);
};