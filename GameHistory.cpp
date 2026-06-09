#include "GameHistory.h"

void GameHistory::record(const Move& move, const Board& boardBefore, Color playerColor) {
    records.push_back({move, boardBefore, playerColor});
}

void GameHistory::clear() {
    records.clear();
}