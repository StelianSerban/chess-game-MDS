#include "GameLogic.h"
#include "MoveGen.h"
#include <cstdlib>

bool GameLogic::isInCheck(const Board& board, Color color) {
    int kingRow = -1, kingCol = -1;
    char kingPiece = (color == Color::White) ? 'K' : 'k';
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (board.grid[r][c] == kingPiece) { kingRow = r; kingCol = c; }

    if (kingRow == -1) return false;

    Color enemy = (color == Color::White) ? Color::Black : Color::White;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (Board::pieceColor(board.grid[r][c]) != enemy) continue;
            auto moves = MoveGen::getRawMoves(board, r, c);
            for (auto& m : moves)
                if (m.toRow == kingRow && m.toCol == kingCol)
                    return true;
        }
    }
    return false;
}

std::vector<Move> GameLogic::getLegalMoves(const Board& board, int row, int col) {
    std::vector<Move> legal;
    Color myColor = Board::pieceColor(board.grid[row][col]);
    auto rawMoves = MoveGen::getRawMoves(board, row, col);

    for (auto& move : rawMoves) {
        Board copy = board;
        copy.applyMove(move);

        bool valid = !isInCheck(copy, myColor);

        char piece = board.grid[row][col];
        if ((piece == 'K' || piece == 'k') && abs(move.toCol - move.fromCol) == 2) {
            int midCol = (move.fromCol + move.toCol) / 2;
            Board mid = board;
            mid.applyMove({row, col, row, midCol});
            if (isInCheck(mid, myColor)) valid = false;
            if (isInCheck(board, myColor)) valid = false;
        }

        if (valid)
            legal.push_back(move);
    }
    return legal;
}

bool GameLogic::isCheckmate(const Board& board, Color color) {
    if (!isInCheck(board, color)) return false;
    return isStalemate(board, color);
}

bool GameLogic::isStalemate(const Board& board, Color color) {
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (Board::pieceColor(board.grid[r][c]) == color)
                if (!getLegalMoves(board, r, c).empty())
                    return false;
    return true;
}