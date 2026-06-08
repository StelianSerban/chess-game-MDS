#include "MoveGen.h"
#include <cctype>

std::vector<Move> MoveGen::getRawMoves(const Board& board, int row, int col) {
    char piece = board.grid[row][col];
    char lower = tolower(piece);

    if (lower == 'p') return getPawnMoves(board, row, col);
    if (lower == 'n') return getKnightMoves(board, row, col);
    if (lower == 'b') return getBishopMoves(board, row, col);
    if (lower == 'r') return getRookMoves(board, row, col);
    if (lower == 'q') return getQueenMoves(board, row, col);
    if (lower == 'k') return getKingMoves(board, row, col);
    return {};
}

void MoveGen::addSlidingMoves(const Board& board, int row, int col,
                               const int dirs[][2], int numDirs,
                               std::vector<Move>& moves) {
    Color myColor = Board::pieceColor(board.grid[row][col]);
    for (int i = 0; i < numDirs; i++) {
        int r = row + dirs[i][0];
        int c = col + dirs[i][1];
        while (Board::inBounds(r, c)) {
            char target = board.grid[r][c];
            if (target == ' ') {
                moves.push_back({row, col, r, c});
            } else {
                if (Board::pieceColor(target) != myColor)
                    moves.push_back({row, col, r, c});
                break;
            }
            r += dirs[i][0];
            c += dirs[i][1];
        }
    }
}

std::vector<Move> MoveGen::getPawnMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    char piece   = board.grid[row][col];
    bool isWhite = Board::isWhitePiece(piece);
    int  dir     = isWhite ? -1 : 1;
    int  startRow = isWhite ? 6 : 1;

    // Inainte 1
    int nr = row + dir;
    if (Board::inBounds(nr, col) && board.grid[nr][col] == ' ') {
        moves.push_back({row, col, nr, col});
        // Inainte 2
        if (row == startRow && board.grid[row + 2 * dir][col] == ' ')
            moves.push_back({row, col, row + 2 * dir, col});
    }

    // Atac diagonal normal
    for (int dc : {-1, 1}) {
        int nc = col + dc;
        if (Board::inBounds(nr, nc)) {
            char target = board.grid[nr][nc];
            if (target != ' ' && Board::pieceColor(target) != Board::pieceColor(piece))
                moves.push_back({row, col, nr, nc});
        }
    }

    // En passant
    for (int dc : {-1, 1}) {
        int nc = col + dc;
        if (nr == board.enPassantRow && nc == board.enPassantCol)
            moves.push_back({row, col, nr, nc});
    }

    return moves;
}

std::vector<Move> MoveGen::getKnightMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    Color myColor = Board::pieceColor(board.grid[row][col]);
    const int jumps[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (auto& j : jumps) {
        int r = row + j[0], c = col + j[1];
        if (Board::inBounds(r, c) && Board::pieceColor(board.grid[r][c]) != myColor)
            moves.push_back({row, col, r, c});
    }
    return moves;
}

std::vector<Move> MoveGen::getBishopMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    const int dirs[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
    addSlidingMoves(board, row, col, dirs, 4, moves);
    return moves;
}

std::vector<Move> MoveGen::getRookMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    const int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    addSlidingMoves(board, row, col, dirs, 4, moves);
    return moves;
}

std::vector<Move> MoveGen::getQueenMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    const int dirs[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    addSlidingMoves(board, row, col, dirs, 8, moves);
    return moves;
}

std::vector<Move> MoveGen::getKingMoves(const Board& board, int row, int col) {
    std::vector<Move> moves;
    Color myColor = Board::pieceColor(board.grid[row][col]);
    const int dirs[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    for (auto& d : dirs) {
        int r = row + d[0], c = col + d[1];
        if (Board::inBounds(r, c) && Board::pieceColor(board.grid[r][c]) != myColor)
            moves.push_back({row, col, r, c});
    }

    // Rocada
    bool isWhite = (myColor == Color::White);
    bool kingMoved = isWhite ? board.whiteKingMoved : board.blackKingMoved;
    int  kingRow   = isWhite ? 7 : 0;

    if (!kingMoved && row == kingRow && col == 4) {
        bool rookAMoved = isWhite ? board.whiteRookAMoved : board.blackRookAMoved;
        bool rookHMoved = isWhite ? board.whiteRookHMoved : board.blackRookHMoved;

        // Rocada mica (coloana H)
        if (!rookHMoved &&
            board.grid[kingRow][5] == ' ' &&
            board.grid[kingRow][6] == ' ')
        {
            moves.push_back({row, col, kingRow, 6});
        }

        // Rocada mare (coloana A)
        if (!rookAMoved &&
            board.grid[kingRow][1] == ' ' &&
            board.grid[kingRow][2] == ' ' &&
            board.grid[kingRow][3] == ' ')
        {
            moves.push_back({row, col, kingRow, 2});
        }
    }

    return moves;
}