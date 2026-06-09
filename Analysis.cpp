#include "Analysis.h"
#include "Bot.h"
#include "GameLogic.h"
#include <climits>
#include <cctype>

MoveQuality Analysis::classify(int loss, int evalBest, int evalAfter) {
    
    if (evalBest  >= 90000)  return MoveQuality::Blunder; // mat ratat
    if (evalAfter >= 90000)  return MoveQuality::Good;    // mat dat
    if (loss >= 500)         return MoveQuality::Blunder;  // 5 pioni
    if (loss >= 250)         return MoveQuality::Mistake;  // 2.5 pioni
    if (loss >= 100)         return MoveQuality::Inaccuracy; // 1 pion
    return MoveQuality::Good;
}

std::string Analysis::toNotation(const Move& move) {
    std::string s;
    s += (char)('a' + move.fromCol);
    s += (char)('8' - move.fromRow);
    s += '-';
    s += (char)('a' + move.toCol);
    s += (char)('8' - move.toRow);
    return s;
}

std::vector<MoveAnalysis> Analysis::analyze(
    const GameHistory& history,
    Color humanColor,
    int depth)
{
    std::vector<MoveAnalysis> results;

    for (int i = 0; i < (int)history.records.size(); i++)
    {
        const MoveRecord& rec = history.records[i];

        if (humanColor != Color::None && rec.playerColor != humanColor)
            continue;

        Color mover = rec.playerColor;

        // Scorul mutarii jucate si cel mai bun scor posibil
        // din aceeasi perspectiva si cu aceeasi functie
        int evalAfter = Bot::scoreMoveWithMinimax(rec.boardBefore, rec.move,
                                                   mover, depth);

        auto [bestMove, evalBest] = Bot::getBestMoveWithScore(rec.boardBefore,
                                                               mover, depth);

        // Evaluare statica pentru afisare
        int evalBefore = Bot::getEval(rec.boardBefore, mover);

        // Pierderea — ambele din aceeasi perspectiva, mereu >= 0
        int loss = evalBest - evalAfter;

        MoveAnalysis ma;
        ma.moveIndex   = i;
        ma.playedMove  = rec.move;
        ma.bestMove    = bestMove;
        ma.boardBefore = rec.boardBefore;
        ma.playerColor = mover;
        ma.evalBefore  = evalBefore;
        ma.evalAfter   = evalAfter;
        ma.evalBest    = evalBest;
        ma.loss        = loss;
        ma.quality     = classify(loss, evalBest, evalAfter);
        ma.notation    = toNotation(rec.move);

        printf("Mutare %d: %s | evalAfter=%d | evalBest=%d | loss=%d\n",
               i, toNotation(rec.move).c_str(), evalAfter, evalBest, loss);

        results.push_back(ma);
    }

    return results;
}