#pragma once
#include <vector>
#include "Move.h"

enum class Color { White, Black, None };

class Board {
public:
    char grid[8][8];
    Color currentTurn;

    // En passant
    int enPassantRow = -1;
    int enPassantCol = -1;

    // Rocada - flag-uri daca au mutat
    bool whiteKingMoved  = false;
    bool blackKingMoved  = false;
    bool whiteRookAMoved = false; // coloana 0
    bool whiteRookHMoved = false; // coloana 7
    bool blackRookAMoved = false;
    bool blackRookHMoved = false;

    Board();
    void reset();

    bool applyMove(const Move& move);
    void undoMove(const Move& move, char capturedPiece,
                  int prevEnPassantRow, int prevEnPassantCol,
                  bool prevWhiteKingMoved,  bool prevBlackKingMoved,
                  bool prevWhiteRookAMoved, bool prevWhiteRookHMoved,
                  bool prevBlackRookAMoved, bool prevBlackRookHMoved);

    static Color pieceColor(char piece);
    static bool isWhitePiece(char piece);
    static bool isBlackPiece(char piece);
    static bool inBounds(int row, int col);
};