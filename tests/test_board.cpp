#include <gtest/gtest.h>
#include "../Board.h"

TEST(BoardTest, ResetInitializesCorrectly) {
    Board b;
    EXPECT_EQ(b.grid[0][0], 'r');
    EXPECT_EQ(b.grid[7][4], 'K');
    EXPECT_EQ(b.grid[3][3], ' ');
    EXPECT_EQ(b.currentTurn, Color::White);
}

TEST(BoardTest, PieceColorDetection) {
    EXPECT_EQ(Board::pieceColor('K'), Color::White);
    EXPECT_EQ(Board::pieceColor('k'), Color::Black);
    EXPECT_EQ(Board::pieceColor(' '), Color::None);
}

TEST(BoardTest, ApplyMoveUpdateGrid) {
    Board b;
    b.applyMove({6, 4, 4, 4}); // pion alb e2-e4
    EXPECT_EQ(b.grid[4][4], 'P');
    EXPECT_EQ(b.grid[6][4], ' ');
    EXPECT_EQ(b.currentTurn, Color::Black);
}

TEST(BoardTest, EnPassantSetAfterDoublePush) {
    Board b;
    b.applyMove({6, 4, 4, 4}); // e2-e4
    EXPECT_EQ(b.enPassantRow, 5);
    EXPECT_EQ(b.enPassantCol, 4);
}

TEST(BoardTest, EnPassantResetAfterOtherMove) {
    Board b;
    b.applyMove({6, 4, 4, 4}); // e2-e4
    b.applyMove({1, 0, 2, 0}); // a7-a6
    EXPECT_EQ(b.enPassantRow, -1);
}