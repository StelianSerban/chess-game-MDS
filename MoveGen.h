#pragma once
#include <vector>
#include "Board.h"
#include "Move.h"

class MoveGen {
public:
    // Returneaza toate mutarile posibile (fara filtrare sah)
    static std::vector<Move> getRawMoves(const Board& board, int row, int col);

private:
    static void addSlidingMoves(const Board& board, int row, int col,
                                const int dirs[][2], int numDirs,
                                std::vector<Move>& moves);

    static std::vector<Move> getPawnMoves(const Board& board, int row, int col);
    static std::vector<Move> getKnightMoves(const Board& board, int row, int col);
    static std::vector<Move> getBishopMoves(const Board& board, int row, int col);
    static std::vector<Move> getRookMoves(const Board& board, int row, int col);
    static std::vector<Move> getQueenMoves(const Board& board, int row, int col);
    static std::vector<Move> getKingMoves(const Board& board, int row, int col);
};