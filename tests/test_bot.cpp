// tests/test_bot.cpp
#include <gtest/gtest.h>
#include "../Board.h"
#include "../Bot.h"
#include "../GameLogic.h"

// Helper: tabla goala
static Board emptyBoard() {
    Board b;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            b.grid[r][c] = ' ';
    return b;
}

TEST(BotTest, DoesNotMoveIntoCapture) {
    Board b = emptyBoard();
    b.grid[7][4] = 'K';
    b.grid[0][4] = 'k';
    b.grid[7][0] = 'R';
    b.grid[4][3] = 'p';
    b.grid[4][5] = 'p';
    b.currentTurn = Color::White;

    Move best = Bot::getBestMove(b, Color::White, 3);
    EXPECT_FALSE(best.toRow == 4 && best.toCol == 4);
}

TEST(BotTest, DoesNotSacrificeQueenForPawn) {
    Board b = emptyBoard();
    b.grid[7][4] = 'K';
    b.grid[0][4] = 'k';
    b.grid[7][3] = 'Q';
    b.grid[3][3] = 'p';
    b.grid[4][2] = 'r';
    b.currentTurn = Color::White;

    Move best = Bot::getBestMove(b, Color::White, 3);
    EXPECT_FALSE(best.toRow == 3 && best.toCol == 3);
}

TEST(BotTest, DoesNotSacrificeRookForPawn) {
    Board b = emptyBoard();
    b.grid[7][4] = 'K';
    b.grid[0][4] = 'k';
    b.grid[7][0] = 'R';
    b.grid[3][0] = 'p';
    b.grid[4][1] = 'r';
    b.currentTurn = Color::White;

    Move best = Bot::getBestMove(b, Color::White, 3);
    EXPECT_FALSE(best.toRow == 3 && best.toCol == 0);
}