#pragma once
#include "Board.h"
#include "Move.h"
#include "GameHistory.h"
#include <vector>
#include <string>

enum class MoveQuality {
    Good,
    Inaccuracy,  // 50-150 pts pierdut
    Mistake,     // 150-300 pts pierdut
    Blunder      // 300+ pts pierdut
};

struct MoveAnalysis {
    int          moveIndex;
    Move         playedMove;
    Move         bestMove;
    Board        boardBefore;
    Color        playerColor;
    int          evalBefore;   // evaluarea inainte de mutare
    int          evalAfter;    // evaluarea dupa mutarea jucata
    int          evalBest;     // evaluarea dupa cea mai buna mutare
    int          loss;         // diferenta evalBefore - evalAfter
    MoveQuality  quality;
    std::string  notation;     // ex: "e2-e4"
};

class Analysis {
public:
static std::string toNotationPublic(const Move& move) { return toNotation(move); }
    static std::vector<MoveAnalysis> analyze(
        const GameHistory& history,
        Color humanColor,
        int depth
    );

private:
    static MoveQuality classify(int loss, int evalBest, int evalAfter);
    static std::string toNotation(const Move& move);
};