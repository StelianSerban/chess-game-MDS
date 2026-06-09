#include "board.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <cassert>
#include <sstream>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

void print_move(Move m)
{
    std::cout << "WATCH OUT!!!  ";
    int from_file = m.from() % 8;
    int from_rank = m.from() / 8;
    int to_file = m.to() % 8;
    int to_rank = m.to() / 8;

    std::cout << (char)('a' + from_file) << from_rank + 1
              << (char)('a' + to_file) << to_rank + 1;

    if (m.is_promotion())
    {
        int flag = m.flag();
        char promo_char = ' ';
        if (flag == PRO_QUEEN || flag == PRO_QUEEN_CAP)
            promo_char = 'q';
        else if (flag == PRO_ROOK || flag == PRO_ROOK_CAP)
            promo_char = 'r';
        else if (flag == PRO_BISHOP || flag == PRO_BISHOP_CAP)
            promo_char = 'b';
        else if (flag == PRO_KNIGHT || flag == PRO_KNIGHT_CAP)
            promo_char = 'n';
        std::cout << "=" << promo_char;
    }
    std::cout << std::endl;
}

Move string_to_move(std::string move_str, Board &board)
{
    // 1. Generate all currently legal moves
    MoveList list = board.generate_moves();

    // 2. Parse the coordinates from the string (e.g., "e2e4")
    // 'a' = 97 in ASCII. '1' is the bottom, so we invert the rank.
    int from_sq = (move_str[0] - 'a') + (move_str[1] - '1') * 8;
    int to_sq = (move_str[2] - 'a') + (move_str[3] - '1') * 8;

    std::cout << "Parsed move: from " << from_sq << " to " << to_sq << std::endl;

    // 3. Check for promotion character (e.g., "e7e8q")
    char promo_char = (move_str.length() == 5) ? move_str[4] : '\0';

    // 4. Find the matching move in our generated list
    for (int i = 0; i < list.count; i++)
    {
        Move m = list.moves[i];

        if (m.from() == from_sq && m.to() == to_sq)
        {
            // If there's a promotion, ensure the flags match the requested piece
            if (promo_char)
            {
                MoveFlag f = (MoveFlag)m.flag();
                if (promo_char == 'q' && (f == PRO_QUEEN || f == PRO_QUEEN_CAP))
                    return m;
                if (promo_char == 'r' && (f == PRO_ROOK || f == PRO_ROOK_CAP))
                    return m;
                if (promo_char == 'b' && (f == PRO_BISHOP || f == PRO_BISHOP_CAP))
                    return m;
                if (promo_char == 'n' && (f == PRO_KNIGHT || f == PRO_KNIGHT_CAP))
                    return m;
                continue; // Wrong promotion piece, keep looking
            }

            // If no promotion, this is our move!
            return m;
        }
    }

    std::cout << "ERROR: Move " << move_str << " not found in generated moves! as " << board.side_to_move << std::endl;

    // Return an "Illegal/Null" move if no match found
    return Move();
}

std::string move_to_string(Move m)
{
    // 1. Extract the raw square integers (0-63)
    int from = m.from();
    int to = m.to();

    // 2. Convert 'from' square to coordinates
    // File: 0 -> 'a', 1 -> 'b', etc.
    // Rank: 0 -> '8', 7 -> '1' (assuming 0 is top-left a8)
    char from_file = 'a' + (from % 8);
    char from_rank = '1' + (from / 8);

    // 3. Convert 'to' square to coordinates
    char to_file = 'a' + (to % 8);
    char to_rank = '1' + (to / 8);

    std::string move_str = "";
    move_str += from_file;
    move_str += from_rank;
    move_str += to_file;
    move_str += to_rank;

    // 4. Handle Promotions (UCI requires a lowercase letter at the end)
    MoveFlag f = (MoveFlag)m.flag();
    if (f >= 8)
    { // Any flag 8 or higher is a promotion in your enum
        if (f == PRO_QUEEN || f == PRO_QUEEN_CAP)
            move_str += 'q';
        else if (f == PRO_ROOK || f == PRO_ROOK_CAP)
            move_str += 'r';
        else if (f == PRO_BISHOP || f == PRO_BISHOP_CAP)
            move_str += 'b';
        else if (f == PRO_KNIGHT || f == PRO_KNIGHT_CAP)
            move_str += 'n';
    }

    return move_str;
}

constexpr int INF = 999999;
constexpr int MATE_SCORE = 100000;
constexpr int DELTA_MARGIN = 200;
TranspositionTable tt;

uint64_t nodes_searched = 0;
void reset_nodes() { nodes_searched = 0; }

// Piece values for MVV-LVA
static const int PIECE_VALUES[6] = {100, 320, 330, 500, 900, 20000};

// ============================================================================
// QUIESCENCE SEARCH (no TT - too many entries, low value)
// ============================================================================

int quiescence(Board &board, int alpha, int beta)
{
    if (board.count_reps() >= 2)
        return 0;

    int stand_pat = board.evaluate();

    if (stand_pat >= beta)
        return beta;

    if (stand_pat + DELTA_MARGIN + 900 < alpha)
        return alpha;

    if (stand_pat > alpha)
        alpha = stand_pat;

    MoveList moves = board.generate_captures();

    // MVV-LVA ordering
    int move_scores[256];
    for (int i = 0; i < moves.size(); i++)
    {
        Move m = moves.moves[i];
        int victim = board.piece_on(m.to());
        int attacker = board.piece_on(m.from());
        int victim_val = (victim >= 0) ? PIECE_VALUES[victim % 6] : 0;
        int attacker_val = (attacker >= 0) ? PIECE_VALUES[attacker % 6] : 0;
        move_scores[i] = victim_val * 10 - attacker_val;
    }

    // Selection sort
    for (int i = 0; i < moves.size() - 1; i++)
    {
        int best_idx = i;
        for (int j = i + 1; j < moves.size(); j++)
        {
            if (move_scores[j] > move_scores[best_idx])
                best_idx = j;
        }
        if (best_idx != i)
        {
            std::swap(moves.moves[i], moves.moves[best_idx]);
            std::swap(move_scores[i], move_scores[best_idx]);
        }
    }

    for (int i = 0; i < moves.size(); i++)
    {
        board.make_move(moves.moves[i]);

        if (board.is_in_check(1 - board.side_to_move))
        {
            board.undo_move();
            continue;
        }

        int score = -quiescence(board, -beta, -alpha);
        board.undo_move();

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

// ============================================================================
// NEGAMAX WITH TRANSPOSITION TABLE
// ============================================================================

int negamax(Board &board, int depth, int alpha, int beta)
{
    nodes_searched++;

    // If position appears 2+ times already, third time = draw
    if (board.count_reps() >= 2)
    {
        return 0; // Draw by repetition
    }

    // Probe transposition table
    TTEntry *tt_entry = tt.probe(board.hash);

    // TT cutoff: if we have a valid entry at sufficient depth
    if (tt_entry && tt_entry->depth >= depth)
    {
        int tt_score = tt_entry->score;

        // Adjust mate scores (they're relative to root)
        if (tt_score > MATE_SCORE - 100)
            tt_score -= (100 - depth);
        else if (tt_score < -MATE_SCORE + 100)
            tt_score += (100 - depth);

        if (tt_entry->flag == TT_EXACT)
            return tt_score;
        else if (tt_entry->flag == TT_LOWER && tt_score >= beta)
            return beta;
        else if (tt_entry->flag == TT_UPPER && tt_score <= alpha)
            return alpha;
    }

    // Leaf node - quiescence search
    if (depth == 0)
        return quiescence(board, alpha, beta);

    MoveList moves = board.generate_moves();

    // No legal moves
    if (moves.size() == 0)
    {
        // This shouldn't happen with proper generation, but safety check
        bool has_legal = false;
        for (int i = 0; i < moves.size(); i++)
        {
            board.make_move(moves.moves[i]);
            if (!board.is_in_check(1 - board.side_to_move))
                has_legal = true;
            board.undo_move();
            if (has_legal)
                break;
        }

        if (!has_legal)
        {
            if (board.is_in_check(board.side_to_move))
                return -MATE_SCORE + (100 - depth);
            return 0;
        }
    }

    // ========== MOVE ORDERING ==========

    int move_scores[256];
    Move tt_best_move = tt_entry ? tt_entry->best_move : Move();

    for (int i = 0; i < moves.size(); i++)
    {
        Move m = moves.moves[i];
        int score = 0;

        // 1. TT move gets highest priority
        if (m.data == tt_best_move.data)
        {
            score = 10000000;
        }
        // 2. Captures (MVV-LVA)
        else if (m.is_capture())
        {
            int victim = board.piece_on(m.to());
            int attacker = board.piece_on(m.from());
            int victim_val = (victim >= 0) ? PIECE_VALUES[victim % 6] : 0;
            int attacker_val = (attacker >= 0) ? PIECE_VALUES[attacker % 6] : 0;
            score = 1000000 + victim_val * 10 - attacker_val;
        }
        // 3. Promotions
        else if (m.is_promotion())
        {
            score = 500000;
        }
        // 4. Quiet moves (no score, sorted last)
        else
        {
            score = 0;
        }

        move_scores[i] = score;
    }

    // Selection sort
    for (int i = 0; i < moves.size() - 1; i++)
    {
        int best_idx = i;
        for (int j = i + 1; j < moves.size(); j++)
        {
            if (move_scores[j] > move_scores[best_idx])
                best_idx = j;
        }
        if (best_idx != i)
        {
            std::swap(moves.moves[i], moves.moves[best_idx]);
            std::swap(move_scores[i], move_scores[best_idx]);
        }
    }

    // ========== SEARCH ==========

    int best_score = -INF;
    Move best_move;
    int flag = TT_UPPER; // Assume upper bound until we find better
    bool has_legal = false;

    for (int i = 0; i < moves.size(); i++)
    {

        board.make_move(moves.moves[i]);

        // ========== PUSH REPETITION TABLE ==========
        board.push_rep();

        // Skip illegal moves
        if (board.is_in_check(1 - board.side_to_move))
        {
            board.undo_move();
            board.pop_rep();
            continue;
        }

        has_legal = true;

        int score = -negamax(board, depth - 1, -beta, -alpha);

        board.undo_move();
        board.pop_rep();

        if (score > best_score)
        {
            best_score = score;
            best_move = moves.moves[i];
        }

        if (score > alpha)
        {
            alpha = score;
            flag = TT_EXACT; // This is a PV node
        }

        if (alpha >= beta)
        {
            flag = TT_LOWER; // This is a cut-off (beta cutoff)
            break;
        }
    }

    // No legal moves = checkmate or stalemate
    if (!has_legal)
    {
        if (board.is_in_check(board.side_to_move))
            return -MATE_SCORE + (100 - depth);
        return 0;
    }

    // Store in transposition table
    // Adjust mate scores to be relative to root
    int store_score = best_score;
    if (store_score > MATE_SCORE - 100)
        store_score += (100 - depth);
    else if (store_score < -MATE_SCORE + 100)
        store_score -= (100 - depth);

    tt.store(board.hash, store_score, depth, flag, best_move);

    return best_score;
}

// ============================================================================
// GET BEST MOVE
// ============================================================================

Move get_best_move(Board &board, int depth, int *out_score = nullptr)
{
    reset_nodes();

    auto start_time = std::chrono::high_resolution_clock::now();

    MoveList moves = board.generate_moves();
    Move best_move;
    bool found_legal = false;
    int alpha = -INF;
    int beta = INF;

    // Get TT move for move ordering at root
    Move tt_move = tt.get_best_move(board.hash);

    // Score moves for ordering
    int move_scores[256];
    for (int i = 0; i < moves.size(); i++)
    {
        Move m = moves.moves[i];
        int score = 0;

        if (m.data == tt_move.data)
        {
            score = 10000000;
        }
        else if (m.is_capture())
        {
            int victim = board.piece_on(m.to());
            int attacker = board.piece_on(m.from());
            int victim_val = (victim >= 0) ? PIECE_VALUES[victim % 6] : 0;
            int attacker_val = (attacker >= 0) ? PIECE_VALUES[attacker % 6] : 0;
            score = 1000000 + victim_val * 10 - attacker_val;
        }
        else if (m.is_promotion())
        {
            score = 500000;
        }

        move_scores[i] = score;
    }

    // Sort
    for (int i = 0; i < moves.size() - 1; i++)
    {
        int best_idx = i;
        for (int j = i + 1; j < moves.size(); j++)
        {
            if (move_scores[j] > move_scores[best_idx])
                best_idx = j;
        }
        if (best_idx != i)
        {
            std::swap(moves.moves[i], moves.moves[best_idx]);
            std::swap(move_scores[i], move_scores[best_idx]);
        }
    }

    for (int i = 0; i < moves.size(); i++)
    {
        board.make_move(moves.moves[i]);

        if (board.is_in_check(1 - board.side_to_move))
        {
            board.undo_move();
            continue;
        }

        found_legal = true;

        int score = -negamax(board, depth - 1, -beta, -alpha);

        board.undo_move();

        if (score > alpha)
        {
            alpha = score;
            best_move = moves.moves[i];
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    double seconds = duration_ms / 1000.0;
    uint64_t nps = (seconds > 0) ? (uint64_t)(nodes_searched / seconds) : 0;

    // Output info
    std::cout << "info depth " << depth
              << " score cp " << alpha
              << " nodes " << nodes_searched
              << " time " << duration_ms
              << " nps " << nps
              << std::endl;

    if (out_score)
        *out_score = alpha;

    if (!found_legal)
    {
        if (board.is_in_check(board.side_to_move))
            std::cout << "Checkmate!" << std::endl;
        else
            std::cout << "Stalemate!" << std::endl;
        return Move();
    }

    return best_move;
}

struct SearchResult
{
    Move best_move;
    int score;
    int depth;
    uint64_t nodes;
    int time_ms;
    int64_t nps;
    Move pv[1024]; // Principal variation
    int pv_length;
};

SearchResult iterative_deepening(Board &board, int max_depth, int time_limit_ms = 0)
{
    SearchResult result;
    result.best_move = Move();
    result.score = 0;
    result.depth = 0;
    result.nodes = 0;
    result.time_ms = 0;
    result.pv_length = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    Move best_move_this_iter = Move();
    int best_score_this_iter = -INF;

    // Iterative deepening loop
    for (int depth = 1; depth <= max_depth; depth++)
    {
        reset_nodes();

        auto iter_start = std::chrono::high_resolution_clock::now();

        // Search at this depth
        MoveList moves = board.generate_moves();

        // Use previous best move for ordering
        Move prev_best = result.best_move;

        // Score and sort moves
        int move_scores[256];
        for (int i = 0; i < moves.size(); i++)
        {
            Move m = moves.moves[i];
            int score = 0;

            // Previous best move gets highest priority
            if (m.data == prev_best.data)
                score = 20000000;
            // TT move
            else if (m.data == tt.get_best_move(board.hash).data)
                score = 10000000;
            // Captures (MVV-LVA)
            else if (m.is_capture())
            {
                int victim = board.piece_on(m.to());
                int attacker = board.piece_on(m.from());
                int victim_val = (victim >= 0) ? PIECE_VALUES[victim % 6] : 0;
                int attacker_val = (attacker >= 0) ? PIECE_VALUES[attacker % 6] : 0;
                score = 1000000 + victim_val * 10 - attacker_val;
            }
            // Promotions
            else if (m.is_promotion())
                score = 500000;

            move_scores[i] = score;
        }

        // Sort
        for (int i = 0; i < moves.size() - 1; i++)
        {
            int best_idx = i;
            for (int j = i + 1; j < moves.size(); j++)
            {
                if (move_scores[j] > move_scores[best_idx])
                    best_idx = j;
            }
            if (best_idx != i)
            {
                std::swap(moves.moves[i], moves.moves[best_idx]);
                std::swap(move_scores[i], move_scores[best_idx]);
            }
        }

        // Search all moves at root
        int alpha = -INF;
        int beta = INF;
        best_move_this_iter = Move();
        best_score_this_iter = -INF;
        bool found_legal = false;

        for (int i = 0; i < moves.size(); i++)
        {
            board.make_move(moves.moves[i]);

            if (board.is_in_check(1 - board.side_to_move))
            {
                board.undo_move();
                continue;
            }

            found_legal = true;

            int score = -negamax(board, depth - 1, -beta, -alpha);

            board.undo_move();

            if (score > best_score_this_iter)
            {
                best_score_this_iter = score;
                best_move_this_iter = moves.moves[i];
            }

            if (score > alpha)
                alpha = score;
        }

        auto iter_end = std::chrono::high_resolution_clock::now();
        auto iter_time = std::chrono::duration_cast<std::chrono::milliseconds>(iter_end - iter_start).count();

        // Check for mate/stalemate
        if (!found_legal)
        {
            if (board.is_in_check(board.side_to_move))
                best_score_this_iter = -MATE_SCORE;
            else
                best_score_this_iter = 0;
            break;
        }

        // Update result with this iteration's best
        result.best_move = best_move_this_iter;
        result.score = best_score_this_iter;
        result.depth = depth;
        result.nodes += nodes_searched;

        auto current_time = std::chrono::high_resolution_clock::now();
        result.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        result.nps = (result.time_ms > 0) ? (result.nodes * 1000 / result.time_ms) : 0;

        // Time management: stop if we've used enough time
        if (time_limit_ms > 0 && result.time_ms >= time_limit_ms)
            break;

        // Early exit if we found a mate
        if (best_score_this_iter > MATE_SCORE - 100 || best_score_this_iter < -MATE_SCORE + 100)
            break;
    }

    return result;
}

#include <iomanip>

void print_search_result(const SearchResult &res)
{
    std::cout << "\n=================================================" << std::endl;
    std::cout << "  SEARCH COMPLETE - Depth " << res.depth << std::endl;
    std::cout << "=================================================" << std::endl;

    // Format Score (Convert centipawns to pawns, e.g., 125 -> +1.25)
    float printable_score = res.score / 100.0f;
    std::string score_prefix = (res.score > 0) ? "+" : "";

    std::cout << std::left << std::setw(15) << "Score:" << score_prefix << std::fixed << std::setprecision(2) << printable_score << std::endl;
    std::cout << std::left << std::setw(15) << "Nodes:" << res.nodes << " (" << (res.nodes / 1000) << "k)" << std::endl;
    std::cout << std::left << std::setw(15) << "NPS:" << res.nps / 1000000 << " MNPS" << std::endl;
    std::cout << std::left << std::setw(15) << "Time:" << res.time_ms << " ms" << std::endl;

    std::cout << "\n=================================================\n"
              << std::endl;
}

int main()
{
    Board board;
    tt.init(256);
    bool first_bs = true;

    int read_fd = open("../gui_to_engine", O_RDONLY);
    int write_fd = open("engine_to_gui", O_WRONLY);
    char buffer[256];

    while (true)
    {
        ssize_t bytes = read(read_fd,
                             buffer,
                             sizeof(buffer));

        if (bytes <= 0)
            break;

        std::cout << "Received move from GUI: " << std::string(buffer, bytes) << std::endl;
        board.make_move(string_to_move(std::string(buffer, bytes), board));

        Move Bestmove = get_best_move(board, 4);
        board.make_move(Bestmove);
        std::string engine_move = move_to_string(Bestmove) + "\n";

        write(write_fd,
              engine_move.c_str(),
              engine_move.size());
    }

    close(read_fd);
    close(write_fd);
    return 0;
}