#include "Board.h"

Board::Board() {
    reset();
}

void Board::reset() {
    char initial[8][8] = {
        {'r','n','b','q','k','b','n','r'},
        {'p','p','p','p','p','p','p','p'},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {' ',' ',' ',' ',' ',' ',' ',' '},
        {'P','P','P','P','P','P','P','P'},
        {'R','N','B','Q','K','B','N','R'}
    };
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            grid[r][c] = initial[r][c];

    currentTurn = Color::White;
}

bool Board::applyMove(const Move& move) {
    grid[move.toRow][move.toCol]     = grid[move.fromRow][move.fromCol];
    grid[move.fromRow][move.fromCol] = ' ';
    currentTurn = (currentTurn == Color::White) ? Color::Black : Color::White;
    return true;
}

void Board::undoMove(const Move& move, char capturedPiece) {
    grid[move.fromRow][move.fromCol] = grid[move.toRow][move.toCol];
    grid[move.toRow][move.toCol]     = capturedPiece;
    currentTurn = (currentTurn == Color::White) ? Color::Black : Color::White;
}

Color Board::pieceColor(char piece) {
    if (piece >= 'A' && piece <= 'Z') return Color::White;
    if (piece >= 'a' && piece <= 'z') return Color::Black;
    return Color::None;
}

bool Board::isWhitePiece(char piece) { return pieceColor(piece) == Color::White; }
bool Board::isBlackPiece(char piece) { return pieceColor(piece) == Color::Black; }
bool Board::inBounds(int row, int col) { return row >= 0 && row < 8 && col >= 0 && col < 8; }