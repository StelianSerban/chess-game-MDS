#pragma once
#include "Board.h"
#include "Move.h"
#include <vector>

struct MoveRecord {
    Move move;
    Board boardBefore;  // starea tablei INAINTE de mutare
    Color playerColor;
};

class GameHistory {
public:
    std::vector<MoveRecord> records;

    void record(const Move& move, const Board& boardBefore, Color playerColor);
    void clear();
};