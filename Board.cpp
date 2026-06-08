#include "Board.h"

Board::Board() { reset(); }

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

    currentTurn     = Color::White;
    enPassantRow    = -1;
    enPassantCol    = -1;
    whiteKingMoved  = false;
    blackKingMoved  = false;
    whiteRookAMoved = false;
    whiteRookHMoved = false;
    blackRookAMoved = false;
    blackRookHMoved = false;
}

bool Board::applyMove(const Move& move) {
    char piece   = grid[move.fromRow][move.fromCol];
    char target  = grid[move.toRow][move.toCol];

    // Reseteaza en passant
    enPassantRow = -1;
    enPassantCol = -1;

    // En passant - captura
    if ((piece == 'P' || piece == 'p') &&
        move.fromCol != move.toCol &&
        target == ' ')
    {
        grid[move.fromRow][move.toCol] = ' ';
    }

    // Seteaza en passant daca pionul avanseaza 2
    if (piece == 'P' && move.fromRow - move.toRow == 2) {
        enPassantRow = move.fromRow - 1;
        enPassantCol = move.fromCol;
    }
    if (piece == 'p' && move.toRow - move.fromRow == 2) {
        enPassantRow = move.fromRow + 1;
        enPassantCol = move.fromCol;
    }

    // Rocada - muta si turnul
    if (piece == 'K' && move.fromCol == 4) {
        if (move.toCol == 6) { // rocada mica alba
            grid[7][5] = 'R'; grid[7][7] = ' ';
        } else if (move.toCol == 2) { // rocada mare alba
            grid[7][3] = 'R'; grid[7][0] = ' ';
        }
        whiteKingMoved = true;
    }
    if (piece == 'k' && move.fromCol == 4) {
        if (move.toCol == 6) { // rocada mica neagra
            grid[0][5] = 'r'; grid[0][7] = ' ';
        } else if (move.toCol == 2) { // rocada mare neagra
            grid[0][3] = 'r'; grid[0][0] = ' ';
        }
        blackKingMoved = true;
    }

    // Actualizeaza flag-uri turn
    if (piece == 'R') {
        if (move.fromCol == 0) whiteRookAMoved = true;
        if (move.fromCol == 7) whiteRookHMoved = true;
    }
    if (piece == 'r') {
        if (move.fromCol == 0) blackRookAMoved = true;
        if (move.fromCol == 7) blackRookHMoved = true;
    }

    // Muta piesa
    grid[move.toRow][move.toCol]     = piece;
    grid[move.fromRow][move.fromCol] = ' ';

    // Promovare - e tratata in main.cpp (UI), aici doar marcam
    // main.cpp va schimba piesa dupa ce userul alege

    currentTurn = (currentTurn == Color::White) ? Color::Black : Color::White;
    return true;
}

void Board::undoMove(const Move& move, char capturedPiece,
                     int prevEnPassantRow, int prevEnPassantCol,
                     bool prevWhiteKingMoved,  bool prevBlackKingMoved,
                     bool prevWhiteRookAMoved, bool prevWhiteRookHMoved,
                     bool prevBlackRookAMoved, bool prevBlackRookHMoved)
{
    char piece = grid[move.toRow][move.toCol];

    grid[move.fromRow][move.fromCol] = piece;
    grid[move.toRow][move.toCol]     = capturedPiece;

    // Undo en passant
    if ((piece == 'P' || piece == 'p') &&
        move.fromCol != move.toCol &&
        capturedPiece == ' ')
    {
        char enemyPawn = (piece == 'P') ? 'p' : 'P';
        grid[move.fromRow][move.toCol] = enemyPawn;
    }

    // Undo rocada
    if (piece == 'K' && move.fromCol == 4) {
        if (move.toCol == 6) { grid[7][7] = 'R'; grid[7][5] = ' '; }
        if (move.toCol == 2) { grid[7][0] = 'R'; grid[7][3] = ' '; }
    }
    if (piece == 'k' && move.fromCol == 4) {
        if (move.toCol == 6) { grid[0][7] = 'r'; grid[0][5] = ' '; }
        if (move.toCol == 2) { grid[0][0] = 'r'; grid[0][3] = ' '; }
    }

    enPassantRow    = prevEnPassantRow;
    enPassantCol    = prevEnPassantCol;
    whiteKingMoved  = prevWhiteKingMoved;
    blackKingMoved  = prevBlackKingMoved;
    whiteRookAMoved = prevWhiteRookAMoved;
    whiteRookHMoved = prevWhiteRookHMoved;
    blackRookAMoved = prevBlackRookAMoved;
    blackRookHMoved = prevBlackRookHMoved;

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