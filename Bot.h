#pragma once
#include "Board.h"
#include "Move.h"

class Bot {
public:
    static Move getBestMove(const Board& board, Color color, int depth);

private:
    static int minimax(Board board, int depth, bool isMaximizing,
                       Color botColor, int alpha, int beta);
    static int evaluate(const Board& board, Color botColor);
    static int pieceValue(char piece);
    static int positionalBonus(char piece, int row, int col, Color botColor);
};