#include <gtest/gtest.h>
#include "../Board.h"
#include "../MoveGen.h"

TEST(MoveGenTest, PawnCanMoveOneOrTwoSquares) {
    Board b;
    auto moves = MoveGen::getRawMoves(b, 6, 4); // pion e2
    EXPECT_EQ(moves.size(), 2); // e3 si e4
}

TEST(MoveGenTest, PawnBlockedCannotMove) {
    Board b;
    b.grid[5][4] = 'p'; // blocat
    auto moves = MoveGen::getRawMoves(b, 6, 4);
    EXPECT_EQ(moves.size(), 0);
}

TEST(MoveGenTest, KnightMovesFromCenter) {
    Board b;
    // Goleste tabla si pune un cal in centru
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.grid[r][c] = ' ';
    b.grid[4][4] = 'N';
    auto moves = MoveGen::getRawMoves(b, 4, 4);
    EXPECT_EQ(moves.size(), 8);
}

TEST(MoveGenTest, KnightMovesFromCorner) {
    Board b;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.grid[r][c] = ' ';
    b.grid[0][0] = 'N';
    auto moves = MoveGen::getRawMoves(b, 0, 0);
    EXPECT_EQ(moves.size(), 2);
}

TEST(MoveGenTest, RookMovesOnEmptyBoard) {
    Board b;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.grid[r][c] = ' ';
    b.grid[4][4] = 'R';
    auto moves = MoveGen::getRawMoves(b, 4, 4);
    EXPECT_EQ(moves.size(), 14); // 7 pe linie + 7 pe coloana
}

TEST(MoveGenTest, CastlingAvailableAtStart) {
    Board b;
    // Goleste piesele dintre rege si turn
    b.grid[7][5] = ' ';
    b.grid[7][6] = ' ';
    auto moves = MoveGen::getRawMoves(b, 7, 4); // regele alb
    bool hasCastle = false;
    for (auto& m : moves)
        if (m.toCol == 6) hasCastle = true;
    EXPECT_TRUE(hasCastle);
}