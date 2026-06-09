#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
#include <random>
#include <cstring>

typedef uint64_t U64;

// Piece indices for bitboards array
enum Piece
{
    WHITE_PAWN = 0,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    ALL_WHITE,
    ALL_BLACK,
    ALL_PIECES,
    PIECE_NB
};

// Move flags (Bits 12-15)
enum MoveFlag
{
    QUIET = 0,
    DOUBLE_PUSH = 1,
    KING_CASTLE = 2,
    QUEEN_CASTLE = 3,
    CAPTURE = 4,
    EN_PASSANT = 5,
    PRO_KNIGHT = 8,
    PRO_BISHOP = 9,
    PRO_ROOK = 10,
    PRO_QUEEN = 11,
    PRO_KNIGHT_CAP = 12,
    PRO_BISHOP_CAP = 13,
    PRO_ROOK_CAP = 14,
    PRO_QUEEN_CAP = 15
};

struct Move
{
    uint16_t data;

    Move() : data(0) {}
    Move(int from, int to, int flag)
    {
        data = (from & 0x3F) | ((to & 0x3F) << 6) | ((flag & 0x0F) << 12);
    }

    inline int from() const { return data & 0x3F; }
    inline int to() const { return (data >> 6) & 0x3F; }
    inline int flag() const { return (data >> 12) & 0x0F; }
    inline bool is_capture() const { return (data >> 14) & 1; }
    inline bool is_promotion() const { return (data >> 15) & 1; }

    bool operator==(const Move &other) const { return data == other.data; }
};

struct MoveList
{
    Move moves[256];
    int count;

    MoveList() : count(0) {}

    void push(Move m) { moves[count++] = m; }

    Move *begin() { return moves; }
    Move *end() { return moves + count; }
    int size() const { return count; }
};

// Information needed to undo a move
struct MoveInfo
{
    Move move;           // The move that was made
    int captured_piece;  // Index of captured piece (-1 if none)
    int prev_ep_square;  // Previous en passant square
    bool prev_castle[4]; // Previous castling rights
    bool was_ep_capture; // Was this an en passant capture?
    int prev_score;      // Score before this move (for undo)
    U64 prev_hash;       // Previous position hash (for undo)
};

// Stack for move history (for undo)
struct MoveStack
{
    static constexpr int MAX_DEPTH = 1024;
    MoveInfo stack[MAX_DEPTH];
    int count;

    MoveStack() : count(0) {}

    void push(const MoveInfo &info) { stack[count++] = info; }
    MoveInfo pop() { return stack[--count]; }
    MoveInfo &top() { return stack[count - 1]; }
    void clear() { count = 0; }
};

// TT Entry flags
enum TTFlag
{
    TT_EXACT, // Exact score (PV node)
    TT_LOWER, // Lower bound (alpha cutoff)
    TT_UPPER  // Upper bound (beta cutoff)
};

// Transposition table entry (16 bytes)
struct TTEntry
{
    U64 hash;       // 8 bytes - position hash (only full hash for simplicity)
    int16_t score;  // 2 bytes - score from search
    uint8_t depth;  // 1 byte - search depth
    uint8_t flag;   // 1 byte - exact/lower/upper
    Move best_move; // 2 bytes - best move found

    // Padding to 16 bytes if needed (currently exactly 14 bytes)
};

class TranspositionTable
{
private:
    TTEntry *table;
    size_t size;
    size_t mask; // For fast index calculation (size - 1, when size is power of 2)

public:
    TranspositionTable() : table(nullptr), size(0), mask(0) {}

    ~TranspositionTable()
    {
        if (table)
            delete[] table;
    }

    // Initialize table with given size in MB
    void init(size_t mb_size)
    {
        if (table)
            delete[] table;

        // Calculate number of entries (each entry is ~16 bytes)
        size_t bytes = mb_size * 1024 * 1024;
        size = bytes / sizeof(TTEntry);

        // Round down to power of 2 for fast indexing
        size_t power_of_2 = 1;
        while (power_of_2 * 2 <= size)
            power_of_2 *= 2;
        size = power_of_2;
        mask = size - 1;

        table = new TTEntry[size];
        clear();
    }

    void clear()
    {
        if (table)
            memset(table, 0, size * sizeof(TTEntry));
    }

    // Get index from hash
    inline size_t index(U64 hash) const
    {
        return hash & mask;
    }

    // Probe the table - returns nullptr if not found or hash mismatch
    TTEntry *probe(U64 hash)
    {
        if (!table)
            return nullptr;

        TTEntry *entry = &table[index(hash)];

        // Check if hash matches (avoid key collisions)
        if (entry->hash == hash)
            return entry;

        return nullptr;
    }

    // Store an entry in the table
    void store(U64 hash, int score, int depth, int flag, Move best_move)
    {
        if (!table)
            return;

        TTEntry *entry = &table[index(hash)];

        // Always replace strategy (simple, but works)
        // More sophisticated: replace if deeper or older
        entry->hash = hash;
        entry->score = (int16_t)score;
        entry->depth = (uint8_t)depth;
        entry->flag = (uint8_t)flag;
        entry->best_move = best_move;
    }

    // Get stored move for move ordering
    Move get_best_move(U64 hash) const
    {
        if (!table)
            return Move();

        const TTEntry *entry = &table[index(hash)];

        if (entry->hash == hash && entry->best_move.data != 0)
            return entry->best_move;

        return Move();
    }

    size_t get_size() const { return size; }
};

// Bitboard helper functions
inline int pop_lsb(U64 &bb)
{
    int idx = __builtin_ctzll(bb);
    bb &= bb - 1; // Clear the LSB
    return idx;
}

inline int count_bits(U64 bb)
{
    return __builtin_popcountll(bb);
}

// File masks for wrap-around prevention
constexpr U64 NOT_A_FILE = 0xfefefefefefefefeULL;
constexpr U64 NOT_H_FILE = 0x7f7f7f7f7f7f7f7fULL;
constexpr U64 NOT_AB_FILE = 0xfcfcfcfcfcfcfcfcULL;
constexpr U64 NOT_GH_FILE = 0x3f3f3f3f3f3f3f3fULL;

// Rank masks
constexpr U64 RANK_2 = 0x000000000000FF00ULL;
constexpr U64 RANK_3 = 0x0000000000FF0000ULL;
constexpr U64 RANK_4 = 0x00000000FF000000ULL;
constexpr U64 RANK_5 = 0x000000FF00000000ULL;
constexpr U64 RANK_6 = 0x0000FF0000000000ULL;
constexpr U64 RANK_7 = 0x00FF000000000000ULL;
constexpr U64 RANK_8 = 0xFF00000000000000ULL;

class Board
{
public:
    U64 bitboards[PIECE_NB];
    int side_to_move;      // 0 = White, 1 = Black
    int en_passant_sq;     // -1 if no en passant
    bool castle_rights[4]; // WK, WQ, BK, BQ
    int score;             // Score from white's perspective

    // Piece-square tables
    int knight_pst[64];
    int pawn_pst[64];
    int bishop_pst[64];
    int rook_pst[64];
    int queen_pst[64];

    // Precomputed attack masks
    U64 knight_masks[64];
    U64 king_masks[64];

    // Move history stack for undo
    MoveStack history;

    // Zobrist random numbers
    static U64 zobrist_piece[12][64]; // [piece][square]
    static U64 zobrist_side;          // Black to move
    static U64 zobrist_castle[16];    // 4 bits = 16 combinations
    static U64 zobrist_ep[8];         // En passant file (0-7)

    U64 hash; // Current position hash

    // Repetition table for draw detection (simple implementation)
    U64 rep_table[1024];
    int rep_index; // Current index in repetition table

    Board();

    // Main move generation - generates ALL moves for current position
    MoveList generate_moves() const;
    MoveList generate_captures() const; // For quiescence search

    // Make and undo moves
    void make_move(Move m);
    void undo_move();

    // Evaluation
    int get_piece_value(int piece, int sq) const;
    void init_score();
    int evaluate() const;

    // Print board for debugging
    void print() const;

    // Get piece on a square (-1 if empty)
    int piece_on(int sq) const;

    // Check detection
    bool is_square_attacked(int sq, int by_side) const;
    bool is_in_check(int color) const;

    // Repetition detection
    void push_rep();        // Push current hash before searching
    void pop_rep();         // Pop after searching
    int count_reps() const; // Count occurrences in current path

private:
    // Bitboard-based pawn move generation (all pawns at once)
    void generate_pawn_moves_bb(MoveList &list) const;

    // Bitboard-based sliding piece move generation
    void generate_bishop_moves_bb(MoveList &list) const;
    void generate_rook_moves_bb(MoveList &list) const;
    void generate_queen_moves_bb(MoveList &list) const;

    // Non-sliding pieces (use precomputed masks)
    void generate_knight_moves_bb(MoveList &list) const;
    void generate_king_moves_bb(MoveList &list) const;

    // Helper for sliding pieces (magic bitboards alternative - simple ray tracing)
    U64 get_bishop_attacks(int sq, U64 occupied) const;
    U64 get_rook_attacks(int sq, U64 occupied) const;

    // Serialize bitboard to move list
    void serialize_moves(U64 attacks, int from_sq, MoveList &list, int quiet_flag = QUIET, int cap_flag = CAPTURE) const;

    // Castling move generation (for quiescence)
    void generate_pawn_captures_bb(MoveList &list) const;
    void generate_knight_captures_bb(MoveList &list) const;
    void generate_bishop_captures_bb(MoveList &list) const;
    void generate_rook_captures_bb(MoveList &list) const;
    void generate_queen_captures_bb(MoveList &list) const;
    void generate_king_captures_bb(MoveList &list) const;

    // Zobrist Hashing (for Transposition Tables)
    void init_zobrist();
    void compute_hash();
};
