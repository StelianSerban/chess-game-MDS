#include <gtest/gtest.h>
#include "../Board.h"
#include "../GameLogic.h"

TEST(GameLogicTest, NoCheckAtStart) {
    Board b;
    EXPECT_FALSE(GameLogic::isInCheck(b, Color::White));
    EXPECT_FALSE(GameLogic::isInCheck(b, Color::Black));
}

TEST(GameLogicTest, DetectsCheckByRook) {
    Board b;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.grid[r][c] = ' ';
    b.grid[0][4] = 'k';
    b.grid[0][0] = 'R'; // turn alb pe acelasi rand
    EXPECT_TRUE(GameLogic::isInCheck(b, Color::Black));
}

TEST(GameLogicTest, NoCheckmateAtStart) {
    Board b;
    EXPECT_FALSE(GameLogic::isCheckmate(b, Color::White));
}

TEST(GameLogicTest, LegalMovesFilterOutCheck) {
    Board b;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.grid[r][c] = ' ';
    // Rege alb in col 4, turn negru pe col 4 - regele nu poate merge pe coloana 4
    b.grid[7][4] = 'K';
    b.grid[0][4] = 'r';
    b.currentTurn = Color::White;
    auto moves = GameLogic::getLegalMoves(b, 7, 4);
    for (auto& m : moves)
        EXPECT_NE(m.toCol, 4);
}

TEST(GameLogicTest, StalemateDetection) {
    Board b;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.grid[r][c] = ' ';
    // Pat clasic: rege negru in colt, atacat pe toate celelalte patrate
    b.grid[0][0] = 'k';
    b.grid[2][1] = 'Q';
    b.grid[1][2] = 'K'; // ... dar nu in sah
    b.currentTurn = Color::Black;
    EXPECT_TRUE(GameLogic::isStalemate(b, Color::Black));
}