#include "board.hpp"

// Piece values in centipawns (100 = 1 pawn)
static const int PIECE_VALUES[12] = {
    100, 320, 330, 500, 900, 20000, // White pieces
    100, 320, 330, 500, 900, 20000  // Black pieces
};

// Static member definitions
U64 Board::zobrist_piece[12][64];
U64 Board::zobrist_side;
U64 Board::zobrist_castle[16];
U64 Board::zobrist_ep[8];

Board::Board() : side_to_move(0), en_passant_sq(-1),
                 knight_pst{-50, -40, -30, -30, -30, -30, -40, -50,
                            -40, -20, 0, 0, 0, 0, -20, -40,
                            -30, 0, 10, 15, 15, 10, 0, -30,
                            -30, 5, 15, 20, 20, 15, 5, -30,
                            -30, 0, 15, 20, 20, 15, 0, -30,
                            -30, 5, 10, 15, 15, 10, 5, -30,
                            -40, -20, 0, 5, 5, 0, -20, -40,
                            -50, -40, -30, -30, -30, -30, -40, -50},

                 pawn_pst{
                     0, 0, 0, 0, 0, 0, 0, 0,
                     5, 10, 10, -20, -20, 10, 10, 5,
                     5, -5, -10, 0, 0, -10, -5, 5,
                     0, 0, 0, 20, 20, 0, 0, 0,
                     5, 5, 10, 25, 25, 10, 5, 5,
                     10, 10, 20, 30, 30, 20, 10, 10,
                     50, 50, 50, 50, 50, 50, 50, 50,
                     0, 0, 0, 0, 0, 0, 0, 0},

                 bishop_pst{
                     -20, -10, -10, -10, -10, -10, -10, -20, // Rank 1 (a1-h1)
                     -10, 5, 0, 0, 0, 0, 5, -10,             // Rank 2
                     -10, 10, 10, 10, 10, 10, 10, -10,       // Rank 3
                     -10, 0, 10, 10, 10, 10, 0, -10,         // Rank 4
                     -10, 5, 5, 10, 10, 5, 5, -10,           // Rank 5
                     -10, 0, 5, 10, 10, 5, 0, -10,           // Rank 6
                     -10, 0, 0, 0, 0, 0, 0, -10,             // Rank 7
                     -20, -10, -10, -10, -10, -10, -10, -20  // Rank 8 (a8-h8)
                 },

                 rook_pst{
                     0, 0, 0, 5, 5, 0, 0, 0,         // Rank 1
                     -5, 0, 0, 0, 0, 0, 0, -5,       // Rank 2
                     -5, 0, 0, 0, 0, 0, 0, -5,       // Rank 3
                     -5, 0, 0, 0, 0, 0, 0, -5,       // Rank 4
                     -5, 0, 0, 0, 0, 0, 0, -5,       // Rank 5
                     -5, 0, 0, 0, 0, 0, 0, -5,       // Rank 6
                     10, 10, 10, 10, 10, 10, 10, 10, // Rank 7 (The "Hungry" Rook)
                     0, 0, 0, 0, 0, 0, 0, 0          // Rank 8

                 },

                 queen_pst{
                     -20, -10, -10, -5, -5, -10, -10, -20, // Rank 1
                     -10, 0, 5, 0, 0, 0, 0, -10,           // Rank 2
                     -10, 5, 5, 5, 5, 5, 0, -10,           // Rank 3
                     -5, 0, 5, 5, 5, 5, 0, -5,             // Rank 4
                     0, 0, 5, 5, 5, 5, 0, -5,              // Rank 5
                     -10, 0, 5, 5, 5, 5, 0, -10,           // Rank 6
                     -10, 0, 0, 0, 0, 0, 0, -10,           // Rank 7
                     -20, -10, -10, -5, -5, -10, -10, -20  // Rank 8
                 }

{
    // Initialize castle rights [WK, WQ, BK, BQ]
    castle_rights[0] = castle_rights[1] = castle_rights[2] = castle_rights[3] = true;

    // Initial position bitboards
    bitboards[WHITE_PAWN] = 0x000000000000FF00ULL;
    bitboards[WHITE_KNIGHT] = 0x0000000000000042ULL;
    bitboards[WHITE_BISHOP] = 0x0000000000000024ULL;
    bitboards[WHITE_ROOK] = 0x0000000000000081ULL;
    bitboards[WHITE_QUEEN] = 0x0000000000000008ULL;
    bitboards[WHITE_KING] = 0x0000000000000010ULL;
    bitboards[ALL_WHITE] = 0x000000000000FFFFULL;

    bitboards[BLACK_PAWN] = 0x00FF000000000000ULL;
    bitboards[BLACK_KNIGHT] = 0x4200000000000000ULL;
    bitboards[BLACK_BISHOP] = 0x2400000000000000ULL;
    bitboards[BLACK_ROOK] = 0x8100000000000000ULL;
    bitboards[BLACK_QUEEN] = 0x0800000000000000ULL;
    bitboards[BLACK_KING] = 0x1000000000000000ULL;
    bitboards[ALL_BLACK] = 0xFFFF000000000000ULL;

    bitboards[ALL_PIECES] = 0xFFFF00000000FFFFULL;

    // Precompute knight attack masks
    for (int sq = 0; sq < 64; sq++)
    {
        U64 bb = 1ULL << sq;
        U64 moves = 0;

        moves |= (bb << 17) & NOT_A_FILE;  // Up 2, Right 1
        moves |= (bb << 15) & NOT_H_FILE;  // Up 2, Left 1
        moves |= (bb << 10) & NOT_AB_FILE; // Up 1, Right 2
        moves |= (bb << 6) & NOT_GH_FILE;  // Up 1, Left 2
        moves |= (bb >> 17) & NOT_H_FILE;  // Down 2, Left 1
        moves |= (bb >> 15) & NOT_A_FILE;  // Down 2, Right 1
        moves |= (bb >> 10) & NOT_GH_FILE; // Down 1, Left 2
        moves |= (bb >> 6) & NOT_AB_FILE;  // Down 1, Right 2

        knight_masks[sq] = moves;
    }

    // Precompute king attack masks
    for (int sq = 0; sq < 64; sq++)
    {
        U64 bb = 1ULL << sq;
        U64 moves = 0;

        moves |= (bb << 8);              // Up
        moves |= (bb >> 8);              // Down
        moves |= (bb << 1) & NOT_A_FILE; // Right
        moves |= (bb >> 1) & NOT_H_FILE; // Left
        moves |= (bb << 9) & NOT_A_FILE; // Up-Right
        moves |= (bb << 7) & NOT_H_FILE; // Up-Left
        moves |= (bb >> 7) & NOT_A_FILE; // Down-Right
        moves |= (bb >> 9) & NOT_H_FILE; // Down-Left

        king_masks[sq] = moves;
    }

    // Initialize evaluation score
    init_score();

    // Initialize Zobrist keys (only once!)
    static bool zobrist_initialized = false;
    if (!zobrist_initialized)
    {
        init_zobrist();
        zobrist_initialized = true;
    }

    // Compute initial hash
    compute_hash();

    // Initialize repetition table
    rep_table[0] = hash; // Store initial position
    rep_index = 0;
}

// ============================================================================
// UNIFIED MOVE GENERATION - Main Entry Point
// ============================================================================

MoveList Board::generate_moves() const
{
    MoveList list;

    // Generate moves for all piece types
    generate_pawn_moves_bb(list);
    generate_knight_moves_bb(list);
    generate_bishop_moves_bb(list);
    generate_rook_moves_bb(list);
    generate_queen_moves_bb(list);
    generate_king_moves_bb(list);

    return list;
}

// ============================================================================
// BITBOARD-BASED PAWN MOVE GENERATION (All Pawns At Once)
// ============================================================================

void Board::generate_pawn_moves_bb(MoveList &list) const
{
    U64 pawns = bitboards[side_to_move == 0 ? WHITE_PAWN : BLACK_PAWN];
    U64 friendlies = bitboards[side_to_move == 0 ? ALL_WHITE : ALL_BLACK];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];
    U64 empty = ~bitboards[ALL_PIECES];

    // En passant target square as bitboard
    U64 ep_bb = (en_passant_sq >= 0) ? (1ULL << en_passant_sq) : 0;

    if (side_to_move == 0) // White to move
    {
        // ========== SINGLE PUSHES ==========
        // Shift all pawns up 1, mask with empty squares
        U64 single_push = (pawns << 8) & empty;

        // Serialize: loop through each target square
        while (single_push)
        {
            int to_sq = pop_lsb(single_push);
            int from_sq = to_sq - 8;

            // Check for promotion (to_sq is on rank 8)
            if (to_sq >= 56) // Rank 8
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN));
                list.push(Move(from_sq, to_sq, PRO_ROOK));
                list.push(Move(from_sq, to_sq, PRO_BISHOP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT));
            }
            else
            {
                list.push(Move(from_sq, to_sq, QUIET));
            }
        }

        // ========== DOUBLE PUSHES ==========
        // Pawns on rank 2, push through empty rank 3 to empty rank 4
        U64 double_push = ((pawns & RANK_2) << 8) & empty; // To rank 3
        double_push = (double_push << 8) & empty;          // To rank 4

        while (double_push)
        {
            int to_sq = pop_lsb(double_push);
            int from_sq = to_sq - 16;
            list.push(Move(from_sq, to_sq, DOUBLE_PUSH));
        }

        // ========== CAPTURES (Diagonal Left - towards A file) ==========
        U64 cap_left = ((pawns & NOT_A_FILE) << 7) & enemies;

        while (cap_left)
        {
            int to_sq = pop_lsb(cap_left);
            int from_sq = to_sq - 7;

            if (to_sq >= 56) // Promotion capture
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN_CAP));
                list.push(Move(from_sq, to_sq, PRO_ROOK_CAP));
                list.push(Move(from_sq, to_sq, PRO_BISHOP_CAP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT_CAP));
            }
            else
            {
                list.push(Move(from_sq, to_sq, CAPTURE));
            }
        }

        // ========== CAPTURES (Diagonal Right - towards H file) ==========
        U64 cap_right = ((pawns & NOT_H_FILE) << 9) & enemies;

        while (cap_right)
        {
            int to_sq = pop_lsb(cap_right);
            int from_sq = to_sq - 9;

            if (to_sq >= 56) // Promotion capture
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN_CAP));
                list.push(Move(from_sq, to_sq, PRO_ROOK_CAP));
                list.push(Move(from_sq, to_sq, PRO_BISHOP_CAP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT_CAP));
            }
            else
            {
                list.push(Move(from_sq, to_sq, CAPTURE));
            }
        }

        // ========== EN PASSANT ==========
        if (ep_bb)
        {
            // Up-left capture: << 7, need NOT_A_FILE to prevent wrap from a-file
            U64 ep_left = ((pawns & NOT_A_FILE) << 7) & ep_bb;
            if (ep_left)
            {
                int to_sq = pop_lsb(ep_left);
                list.push(Move(to_sq - 7, to_sq, EN_PASSANT));
            }

            // Up-right capture: << 9, need NOT_H_FILE to prevent wrap from h-file
            U64 ep_right = ((pawns & NOT_H_FILE) << 9) & ep_bb;
            if (ep_right)
            {
                int to_sq = pop_lsb(ep_right);
                list.push(Move(to_sq - 9, to_sq, EN_PASSANT));
            }
        }
    }
    else // Black to move
    {
        // ========== SINGLE PUSHES ==========
        U64 single_push = (pawns >> 8) & empty;

        while (single_push)
        {
            int to_sq = pop_lsb(single_push);
            int from_sq = to_sq + 8;

            // Promotion (to_sq is on rank 1)
            if (to_sq < 8) // Rank 1
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN));
                list.push(Move(from_sq, to_sq, PRO_ROOK));
                list.push(Move(from_sq, to_sq, PRO_BISHOP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT));
            }
            else
            {
                list.push(Move(from_sq, to_sq, QUIET));
            }
        }

        // ========== DOUBLE PUSHES ==========
        U64 double_push = ((pawns & RANK_7) >> 8) & empty; // To rank 6
        double_push = (double_push >> 8) & empty;          // To rank 5

        while (double_push)
        {
            int to_sq = pop_lsb(double_push);
            int from_sq = to_sq + 16;
            list.push(Move(from_sq, to_sq, DOUBLE_PUSH));
        }

        // ========== CAPTURES (Diagonal Left - towards A file) ==========
        U64 cap_left = ((pawns & NOT_A_FILE) >> 9) & enemies;

        while (cap_left)
        {
            int to_sq = pop_lsb(cap_left);
            int from_sq = to_sq + 9;

            if (to_sq < 8) // Promotion capture
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN_CAP));
                list.push(Move(from_sq, to_sq, PRO_ROOK_CAP));
                list.push(Move(from_sq, to_sq, PRO_BISHOP_CAP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT_CAP));
            }
            else
            {
                list.push(Move(from_sq, to_sq, CAPTURE));
            }
        }

        // ========== CAPTURES (Diagonal Right - towards H file) ==========
        U64 cap_right = ((pawns & NOT_H_FILE) >> 7) & enemies;

        while (cap_right)
        {
            int to_sq = pop_lsb(cap_right);
            int from_sq = to_sq + 7;

            if (to_sq < 8) // Promotion capture
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN_CAP));
                list.push(Move(from_sq, to_sq, PRO_ROOK_CAP));
                list.push(Move(from_sq, to_sq, PRO_BISHOP_CAP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT_CAP));
            }
            else
            {
                list.push(Move(from_sq, to_sq, CAPTURE));
            }
        }

        // ========== EN PASSANT ==========
        if (ep_bb)
        {
            // Down-right capture: >> 7, need NOT_H_FILE to prevent wrap from h-file
            U64 ep_left = ((pawns & NOT_H_FILE) >> 7) & ep_bb;
            if (ep_left)
            {
                int to_sq = pop_lsb(ep_left);
                list.push(Move(to_sq + 7, to_sq, EN_PASSANT));
            }

            // Down-left capture: >> 9, need NOT_A_FILE to prevent wrap from a-file
            U64 ep_right = ((pawns & NOT_A_FILE) >> 9) & ep_bb;
            if (ep_right)
            {
                int to_sq = pop_lsb(ep_right);
                list.push(Move(to_sq + 9, to_sq, EN_PASSANT));
            }
        }
    }
}

// ============================================================================
// BITBOARD-BASED SLIDING PIECE MOVE GENERATION
// ============================================================================

U64 Board::get_bishop_attacks(int sq, U64 occupied) const
{
    U64 attacks = 0;
    int tr = sq / 8;
    int tf = sq % 8;

    // Four diagonal directions
    for (int r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if (occupied & (1ULL << (r * 8 + f)))
            break;
    }
    for (int r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if (occupied & (1ULL << (r * 8 + f)))
            break;
    }
    for (int r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
    {
        attacks |= (1ULL << (r * 8 + f));
        if (occupied & (1ULL << (r * 8 + f)))
            break;
    }
    for (int r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= (1ULL << (r * 8 + f));
        if (occupied & (1ULL << (r * 8 + f)))
            break;
    }

    return attacks;
}

U64 Board::get_rook_attacks(int sq, U64 occupied) const
{
    U64 attacks = 0;
    int tr = sq / 8;
    int tf = sq % 8;

    // Up
    for (int r = tr + 1; r <= 7; r++)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if (occupied & (1ULL << (r * 8 + tf)))
            break;
    }
    // Down
    for (int r = tr - 1; r >= 0; r--)
    {
        attacks |= (1ULL << (r * 8 + tf));
        if (occupied & (1ULL << (r * 8 + tf)))
            break;
    }
    // Right
    for (int f = tf + 1; f <= 7; f++)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if (occupied & (1ULL << (tr * 8 + f)))
            break;
    }
    // Left
    for (int f = tf - 1; f >= 0; f--)
    {
        attacks |= (1ULL << (tr * 8 + f));
        if (occupied & (1ULL << (tr * 8 + f)))
            break;
    }

    return attacks;
}

void Board::serialize_moves(U64 attacks, int from_sq, MoveList &list, int quiet_flag, int cap_flag) const
{
    U64 friendlies = bitboards[side_to_move == 0 ? ALL_WHITE : ALL_BLACK];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];

    attacks &= ~friendlies; // Remove friendly squares

    while (attacks)
    {
        int to_sq = pop_lsb(attacks);
        int flag = (enemies & (1ULL << to_sq)) ? cap_flag : quiet_flag;
        list.push(Move(from_sq, to_sq, flag));
    }
}

void Board::generate_bishop_moves_bb(MoveList &list) const
{
    U64 bishops = bitboards[side_to_move == 0 ? WHITE_BISHOP : BLACK_BISHOP];

    while (bishops)
    {
        int sq = pop_lsb(bishops);
        U64 attacks = get_bishop_attacks(sq, bitboards[ALL_PIECES]);
        serialize_moves(attacks, sq, list);
    }
}

void Board::generate_rook_moves_bb(MoveList &list) const
{
    U64 rooks = bitboards[side_to_move == 0 ? WHITE_ROOK : BLACK_ROOK];

    while (rooks)
    {
        int sq = pop_lsb(rooks);
        U64 attacks = get_rook_attacks(sq, bitboards[ALL_PIECES]);
        serialize_moves(attacks, sq, list);
    }
}

void Board::generate_queen_moves_bb(MoveList &list) const
{
    U64 queens = bitboards[side_to_move == 0 ? WHITE_QUEEN : BLACK_QUEEN];

    while (queens)
    {
        int sq = pop_lsb(queens);
        // Queen = Bishop + Rook attacks
        U64 attacks = get_bishop_attacks(sq, bitboards[ALL_PIECES]) |
                      get_rook_attacks(sq, bitboards[ALL_PIECES]);
        serialize_moves(attacks, sq, list);
    }
}

// ============================================================================
// KNIGHT AND KING MOVE GENERATION (Using Precomputed Masks)
// ============================================================================

void Board::generate_knight_moves_bb(MoveList &list) const
{
    U64 knights = bitboards[side_to_move == 0 ? WHITE_KNIGHT : BLACK_KNIGHT];
    U64 friendlies = bitboards[side_to_move == 0 ? ALL_WHITE : ALL_BLACK];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];

    while (knights)
    {
        int sq = pop_lsb(knights);
        U64 attacks = knight_masks[sq] & ~friendlies;

        while (attacks)
        {
            int to_sq = pop_lsb(attacks);
            int flag = (enemies & (1ULL << to_sq)) ? CAPTURE : QUIET;
            list.push(Move(sq, to_sq, flag));
        }
    }
}

void Board::generate_king_moves_bb(MoveList &list) const
{
    U64 king_bb = bitboards[side_to_move == 0 ? WHITE_KING : BLACK_KING];
    U64 friendlies = bitboards[side_to_move == 0 ? ALL_WHITE : ALL_BLACK];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];

    if (!king_bb)
        return;

    int sq = pop_lsb(king_bb);
    U64 attacks = king_masks[sq] & ~friendlies;

    while (attacks)
    {
        int to_sq = pop_lsb(attacks);
        int flag = (enemies & (1ULL << to_sq)) ? CAPTURE : QUIET;
        list.push(Move(sq, to_sq, flag));
    }

    // ========== CASTLING ==========
    if (side_to_move == 0) // White
    {
        // Kingside castle (e1-g1)
        if (castle_rights[0] &&                   // WK castle right
            !(bitboards[ALL_PIECES] & 0x60ULL) && // f1, g1 empty
            !is_square_attacked(4, 1) &&          // e1 not attacked (not in check)
            !is_square_attacked(5, 1) &&          // f1 not attacked (not passing through check)
            !is_square_attacked(6, 1))            // g1 not attacked (not landing in check)
        {
            list.push(Move(4, 6, KING_CASTLE));
        }
        // Queenside castle (e1-c1)
        if (castle_rights[1] &&                   // WQ castle right
            !(bitboards[ALL_PIECES] & 0x0EULL) && // b1, c1, d1 empty
            !is_square_attacked(4, 1) &&          // e1 not attacked (not in check)
            !is_square_attacked(3, 1) &&          // d1 not attacked (not passing through check)
            !is_square_attacked(2, 1))            // c1 not attacked (not landing in check)
        {
            list.push(Move(4, 2, QUEEN_CASTLE));
        }
    }
    else // Black
    {
        // Kingside castle (e8-g8)
        if (castle_rights[2] &&                                 // BK castle right
            !(bitboards[ALL_PIECES] & 0x6000000000000000ULL) && // f8, g8 empty
            !is_square_attacked(60, 0) &&                       // e8 not attacked
            !is_square_attacked(61, 0) &&                       // f8 not attacked
            !is_square_attacked(62, 0))                         // g8 not attacked
        {
            list.push(Move(60, 62, KING_CASTLE));
        }
        // Queenside castle (e8-c8)
        if (castle_rights[3] &&                                 // BQ castle right
            !(bitboards[ALL_PIECES] & 0x0E00000000000000ULL) && // b8, c8, d8 empty
            !is_square_attacked(60, 0) &&                       // e8 not attacked
            !is_square_attacked(59, 0) &&                       // d8 not attacked
            !is_square_attacked(58, 0))                         // c8 not attacked
        {
            list.push(Move(60, 58, QUEEN_CASTLE));
        }
    }
}

// ============================================================================
// CAPTURE-ONLY MOVE GENERATION (For Quiescence Search)
// ============================================================================

MoveList Board::generate_captures() const
{
    MoveList list;

    generate_pawn_captures_bb(list);
    generate_knight_captures_bb(list);
    generate_bishop_captures_bb(list);
    generate_rook_captures_bb(list);
    generate_queen_captures_bb(list);
    generate_king_captures_bb(list);

    return list;
}

void Board::generate_pawn_captures_bb(MoveList &list) const
{
    U64 pawns = bitboards[side_to_move == 0 ? WHITE_PAWN : BLACK_PAWN];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];
    U64 ep_bb = (en_passant_sq >= 0) ? (1ULL << en_passant_sq) : 0;

    if (side_to_move == 0) // White
    {
        // Capture left (<< 7) - only enemy squares
        U64 cap_left = ((pawns & NOT_A_FILE) << 7) & enemies;
        while (cap_left)
        {
            int to_sq = pop_lsb(cap_left);
            int from_sq = to_sq - 7;
            if (to_sq >= 56) // Promotion capture
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN_CAP));
                list.push(Move(from_sq, to_sq, PRO_ROOK_CAP));
                list.push(Move(from_sq, to_sq, PRO_BISHOP_CAP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT_CAP));
            }
            else
            {
                list.push(Move(from_sq, to_sq, CAPTURE));
            }
        }

        // Capture right (<< 9) - only enemy squares
        U64 cap_right = ((pawns & NOT_H_FILE) << 9) & enemies;
        while (cap_right)
        {
            int to_sq = pop_lsb(cap_right);
            int from_sq = to_sq - 9;
            if (to_sq >= 56)
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN_CAP));
                list.push(Move(from_sq, to_sq, PRO_ROOK_CAP));
                list.push(Move(from_sq, to_sq, PRO_BISHOP_CAP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT_CAP));
            }
            else
            {
                list.push(Move(from_sq, to_sq, CAPTURE));
            }
        }

        // En passant
        if (ep_bb)
        {
            U64 ep_left = ((pawns & NOT_A_FILE) << 7) & ep_bb;
            if (ep_left)
            {
                int to_sq = pop_lsb(ep_left);
                list.push(Move(to_sq - 7, to_sq, EN_PASSANT));
            }

            U64 ep_right = ((pawns & NOT_H_FILE) << 9) & ep_bb;
            if (ep_right)
            {
                int to_sq = pop_lsb(ep_right);
                list.push(Move(to_sq - 9, to_sq, EN_PASSANT));
            }
        }
    }
    else // Black
    {
        // Capture left (>> 9) - only enemy squares
        U64 cap_left = ((pawns & NOT_A_FILE) >> 9) & enemies;
        while (cap_left)
        {
            int to_sq = pop_lsb(cap_left);
            int from_sq = to_sq + 9;
            if (to_sq < 8)
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN_CAP));
                list.push(Move(from_sq, to_sq, PRO_ROOK_CAP));
                list.push(Move(from_sq, to_sq, PRO_BISHOP_CAP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT_CAP));
            }
            else
            {
                list.push(Move(from_sq, to_sq, CAPTURE));
            }
        }

        // Capture right (>> 7) - only enemy squares
        U64 cap_right = ((pawns & NOT_H_FILE) >> 7) & enemies;
        while (cap_right)
        {
            int to_sq = pop_lsb(cap_right);
            int from_sq = to_sq + 7;
            if (to_sq < 8)
            {
                list.push(Move(from_sq, to_sq, PRO_QUEEN_CAP));
                list.push(Move(from_sq, to_sq, PRO_ROOK_CAP));
                list.push(Move(from_sq, to_sq, PRO_BISHOP_CAP));
                list.push(Move(from_sq, to_sq, PRO_KNIGHT_CAP));
            }
            else
            {
                list.push(Move(from_sq, to_sq, CAPTURE));
            }
        }

        // En passant
        if (ep_bb)
        {
            U64 ep_left = ((pawns & NOT_H_FILE) >> 7) & ep_bb;
            if (ep_left)
            {
                int to_sq = pop_lsb(ep_left);
                list.push(Move(to_sq + 7, to_sq, EN_PASSANT));
            }

            U64 ep_right = ((pawns & NOT_A_FILE) >> 9) & ep_bb;
            if (ep_right)
            {
                int to_sq = pop_lsb(ep_right);
                list.push(Move(to_sq + 9, to_sq, EN_PASSANT));
            }
        }
    }
}

void Board::generate_knight_captures_bb(MoveList &list) const
{
    U64 knights = bitboards[side_to_move == 0 ? WHITE_KNIGHT : BLACK_KNIGHT];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];

    while (knights)
    {
        int sq = pop_lsb(knights);
        U64 attacks = knight_masks[sq] & enemies; // Only enemy squares

        while (attacks)
        {
            int to_sq = pop_lsb(attacks);
            list.push(Move(sq, to_sq, CAPTURE));
        }
    }
}

void Board::generate_bishop_captures_bb(MoveList &list) const
{
    U64 bishops = bitboards[side_to_move == 0 ? WHITE_BISHOP : BLACK_BISHOP];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];

    while (bishops)
    {
        int sq = pop_lsb(bishops);
        U64 attacks = get_bishop_attacks(sq, bitboards[ALL_PIECES]) & enemies;

        while (attacks)
        {
            int to_sq = pop_lsb(attacks);
            list.push(Move(sq, to_sq, CAPTURE));
        }
    }
}

void Board::generate_rook_captures_bb(MoveList &list) const
{
    U64 rooks = bitboards[side_to_move == 0 ? WHITE_ROOK : BLACK_ROOK];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];

    while (rooks)
    {
        int sq = pop_lsb(rooks);
        U64 attacks = get_rook_attacks(sq, bitboards[ALL_PIECES]) & enemies;

        while (attacks)
        {
            int to_sq = pop_lsb(attacks);
            list.push(Move(sq, to_sq, CAPTURE));
        }
    }
}

void Board::generate_queen_captures_bb(MoveList &list) const
{
    U64 queens = bitboards[side_to_move == 0 ? WHITE_QUEEN : BLACK_QUEEN];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];

    while (queens)
    {
        int sq = pop_lsb(queens);
        U64 attacks = (get_bishop_attacks(sq, bitboards[ALL_PIECES]) |
                       get_rook_attacks(sq, bitboards[ALL_PIECES])) &
                      enemies;

        while (attacks)
        {
            int to_sq = pop_lsb(attacks);
            list.push(Move(sq, to_sq, CAPTURE));
        }
    }
}

void Board::generate_king_captures_bb(MoveList &list) const
{
    U64 king_bb = bitboards[side_to_move == 0 ? WHITE_KING : BLACK_KING];
    U64 enemies = bitboards[side_to_move == 0 ? ALL_BLACK : ALL_WHITE];

    if (!king_bb)
        return;

    int sq = pop_lsb(king_bb);
    U64 attacks = king_masks[sq] & enemies;

    while (attacks)
    {
        int to_sq = pop_lsb(attacks);
        list.push(Move(sq, to_sq, CAPTURE));
    }

    // Note: No castling in quiescence - king shouldn't castle when under threat
}

// ============================================================================
// EVALUATION
// ============================================================================

// Calculate the full score from scratch (from white's perspective)
void Board::init_score()
{
    score = 0;

    // White pieces - add to score
    for (int p = WHITE_PAWN; p <= WHITE_KING; p++)
    {
        U64 bb = bitboards[p];
        while (bb)
        {
            int sq = pop_lsb(bb);
            score += get_piece_value(p, sq);
        }
    }

    // Black pieces - subtract from score
    for (int p = BLACK_PAWN; p <= BLACK_KING; p++)
    {
        U64 bb = bitboards[p];
        while (bb)
        {
            int sq = pop_lsb(bb);
            score -= get_piece_value(p, sq);
        }
    }
}

int Board::evaluate() const
{
    return side_to_move == 0 ? score : -score;
}

// Get piece value including PST bonus (from white's perspective)
int Board::get_piece_value(int piece, int sq) const
{
    int value = PIECE_VALUES[piece];

    switch (piece)
    {
    case WHITE_BISHOP:
        value += bishop_pst[sq];
        break;
    case WHITE_ROOK:
        value += rook_pst[sq];
        break;
    case WHITE_QUEEN:
        value += queen_pst[sq];
        break;
    case BLACK_BISHOP:
        value += bishop_pst[sq ^ 56]; // Mirror for black
        break;
    case BLACK_ROOK:
        value += rook_pst[sq ^ 56]; // Mirror for black
        break;
    case BLACK_QUEEN:
        value += queen_pst[sq ^ 56]; // Mirror for black
        break;
    case WHITE_KNIGHT:
        value += knight_pst[sq];
        break;
    case BLACK_KNIGHT:
        value += knight_pst[sq ^ 56]; // Mirror for black
        break;
    case WHITE_PAWN:
        value += pawn_pst[sq];
        break;
    case BLACK_PAWN:
        value += pawn_pst[sq ^ 56]; // Mirror for black
        break;
    }

    return value;
}

// ============================================================================
// DEBUG UTILITIES
// ============================================================================

void Board::print() const
{
    const char *piece_chars = "PNBRQKpnbrqk";

    for (int r = 7; r >= 0; r--)
    {
        std::cout << r + 1 << " ";
        for (int f = 0; f < 8; f++)
        {
            int sq = r * 8 + f;
            char c = '.';
            for (int p = 0; p < 12; p++)
            {
                if (bitboards[p] & (1ULL << sq))
                {
                    c = piece_chars[p];
                    break;
                }
            }
            std::cout << c << " ";
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n";
    std::cout << "Side to move: " << (side_to_move == 0 ? "White" : "Black") << std::endl;
}

// ============================================================================
// HELPER: Get piece on square
// ============================================================================

int Board::piece_on(int sq) const
{
    U64 bit = 1ULL << sq;
    for (int p = 0; p < 12; p++)
    {
        if (bitboards[p] & bit)
            return p;
    }
    return -1; // Empty square
}

// ============================================================================
// MAKE MOVE
// ============================================================================

void Board::make_move(Move m)
{
    // Save state for undo
    MoveInfo info;
    info.move = m;
    info.captured_piece = piece_on(m.to());
    info.prev_ep_square = en_passant_sq;
    info.was_ep_capture = false;
    info.prev_score = score;
    info.prev_hash = hash;
    for (int i = 0; i < 4; i++)
        info.prev_castle[i] = castle_rights[i];

    int from = m.from();
    int to = m.to();
    int flag = m.flag();
    U64 from_bit = 1ULL << from;
    U64 to_bit = 1ULL << to;

    int moving_piece = piece_on(from);

    // ========== UPDATE SCORE INCREMENTALLY ==========

    // 1. Remove piece from 'from' square
    if (moving_piece < 6) // White piece
        score -= get_piece_value(moving_piece, from);
    else // Black piece
        score += get_piece_value(moving_piece, from);

    hash ^= zobrist_piece[moving_piece][from]; // Remove moving piece from hash
    if (en_passant_sq >= 0)
        hash ^= zobrist_ep[en_passant_sq % 8]; // Remove old en passant from hash

    // 2. Handle captures (remove captured piece's value)
    if (info.captured_piece >= 0)
    {
        if (info.captured_piece < 6) // White piece captured
            score -= get_piece_value(info.captured_piece, to);
        else // Black piece captured
            score += get_piece_value(info.captured_piece, to);

        bitboards[info.captured_piece] &= ~to_bit;

        hash ^= zobrist_piece[info.captured_piece][to]; // Remove captured piece from hash
    }

    // 3. Handle en passant capture
    if (flag == EN_PASSANT)
    {
        info.was_ep_capture = true;
        int captured_pawn_sq = (side_to_move == 0) ? to - 8 : to + 8;
        int captured_pawn = (side_to_move == 0) ? BLACK_PAWN : WHITE_PAWN;

        // Remove captured pawn's value
        if (side_to_move == 0)
            score += get_piece_value(captured_pawn, captured_pawn_sq); // Black pawn captured, add (subtract negative)
        else
            score -= get_piece_value(captured_pawn, captured_pawn_sq); // White pawn captured, subtract

        bitboards[captured_pawn] &= ~(1ULL << captured_pawn_sq);
        info.captured_piece = captured_pawn;

        hash ^= zobrist_piece[captured_pawn][captured_pawn_sq]; // Remove captured pawn from hash
    }

    // 4. Move the piece
    bitboards[moving_piece] &= ~from_bit;
    bitboards[moving_piece] |= to_bit;

    // 5. Add piece to 'to' square (handle promotions separately)
    if (m.is_promotion())
    {
        // Don't add the pawn - instead add the promoted piece
        bitboards[moving_piece] &= ~to_bit; // Remove pawn from to_sq

        int promo_type = flag >= 12 ? flag - 12 : flag - 8; // 0=n, 1=b, 2=r, 3=q
        int promo_piece = (side_to_move == 0) ? promo_type + 1 : promo_type + 7;

        // Add promoted piece's value
        if (side_to_move == 0)
            score += get_piece_value(promo_piece, to);
        else
            score -= get_piece_value(promo_piece, to);

        bitboards[promo_piece] |= to_bit;

        // Pawn is already XORed out, now XOR in promoted piece
        hash ^= zobrist_piece[promo_piece][to];
    }
    else
    {
        // Add piece back to 'to' square
        if (moving_piece < 6) // White piece
            score += get_piece_value(moving_piece, to);
        else // Black piece
            score -= get_piece_value(moving_piece, to);

        // Place piece at destination
        hash ^= zobrist_piece[moving_piece][to];
    }

    // 6. Handle castling (move the rook and update its PST)
    if (flag == KING_CASTLE)
    {
        if (side_to_move == 0) // White kingside
        {
            score -= get_piece_value(WHITE_ROOK, 7); // Remove from h1
            score += get_piece_value(WHITE_ROOK, 5); // Add to f1
            bitboards[WHITE_ROOK] &= ~(1ULL << 7);
            bitboards[WHITE_ROOK] |= (1ULL << 5);

            hash ^= zobrist_piece[WHITE_ROOK][7]; // Remove from h1
            hash ^= zobrist_piece[WHITE_ROOK][5]; // Place on f1
        }
        else // Black kingside
        {
            score += get_piece_value(BLACK_ROOK, 63); // Remove from h8 (subtract negative = add)
            score -= get_piece_value(BLACK_ROOK, 61); // Add to f8
            bitboards[BLACK_ROOK] &= ~(1ULL << 63);
            bitboards[BLACK_ROOK] |= (1ULL << 61);

            hash ^= zobrist_piece[BLACK_ROOK][63]; // Remove from h8
            hash ^= zobrist_piece[BLACK_ROOK][61]; // Place on f8
        }
    }
    else if (flag == QUEEN_CASTLE)
    {
        if (side_to_move == 0) // White queenside
        {
            score -= get_piece_value(WHITE_ROOK, 0); // Remove from a1
            score += get_piece_value(WHITE_ROOK, 3); // Add to d1
            bitboards[WHITE_ROOK] &= ~(1ULL << 0);
            bitboards[WHITE_ROOK] |= (1ULL << 3);

            hash ^= zobrist_piece[WHITE_ROOK][0]; // Remove from a1
            hash ^= zobrist_piece[WHITE_ROOK][3]; // Place on d1
        }
        else // Black queenside
        {
            score += get_piece_value(BLACK_ROOK, 56); // Remove from a8
            score -= get_piece_value(BLACK_ROOK, 59); // Add to d8
            bitboards[BLACK_ROOK] &= ~(1ULL << 56);
            bitboards[BLACK_ROOK] |= (1ULL << 59);

            hash ^= zobrist_piece[BLACK_ROOK][56]; // Remove from a8
            hash ^= zobrist_piece[BLACK_ROOK][59]; // Place on d8
        }
    }

    // Update castling rights in hash
    int old_castle_key = (castle_rights[0] << 0) |
                         (castle_rights[1] << 1) |
                         (castle_rights[2] << 2) |
                         (castle_rights[3] << 3);

    // Update castling rights
    if (moving_piece == WHITE_KING)
    {
        castle_rights[0] = false;
        castle_rights[1] = false;
    }
    else if (moving_piece == BLACK_KING)
    {
        castle_rights[2] = false;
        castle_rights[3] = false;
    }

    if (from == 0 || to == 0)
        castle_rights[1] = false; // a1
    if (from == 7 || to == 7)
        castle_rights[0] = false; // h1
    if (from == 56 || to == 56)
        castle_rights[3] = false; // a8
    if (from == 63 || to == 63)
        castle_rights[2] = false; // h8

    int new_castle_key = (castle_rights[0] << 0) |
                         (castle_rights[1] << 1) |
                         (castle_rights[2] << 2) |
                         (castle_rights[3] << 3);

    // if castling rights changed, add that change to the zobrist hash
    if (old_castle_key != new_castle_key)
    {
        hash ^= zobrist_castle[old_castle_key];
        hash ^= zobrist_castle[new_castle_key];
    }

    // Update en passant square
    en_passant_sq = -1;
    if (flag == DOUBLE_PUSH)
    {
        en_passant_sq = (side_to_move == 0) ? to - 8 : to + 8;
        hash ^= zobrist_ep[en_passant_sq % 8]; // Add new en passant square to hash
    }

    // Update aggregate bitboards
    bitboards[ALL_WHITE] = 0;
    bitboards[ALL_BLACK] = 0;
    for (int p = WHITE_PAWN; p <= WHITE_KING; p++)
        bitboards[ALL_WHITE] |= bitboards[p];
    for (int p = BLACK_PAWN; p <= BLACK_KING; p++)
        bitboards[ALL_BLACK] |= bitboards[p];
    bitboards[ALL_PIECES] = bitboards[ALL_WHITE] | bitboards[ALL_BLACK];

    // Switch side to move
    side_to_move = 1 - side_to_move;
    // XOR in side to move (switches sides)
    hash ^= zobrist_side;

    // Save to history
    history.push(info);
}

// ============================================================================
// UNDO MOVE (with score restoration)
// ============================================================================

void Board::undo_move()
{
    MoveInfo info = history.pop();
    Move m = info.move;

    // Restore score immediately
    score = info.prev_score;

    // Restore hash
    hash = info.prev_hash;

    // Switch back to the side that made the move
    side_to_move = 1 - side_to_move;

    int from = m.from();
    int to = m.to();
    int flag = m.flag();
    U64 from_bit = 1ULL << from;
    U64 to_bit = 1ULL << to;

    int piece_on_to = piece_on(to);

    // Handle promotions: restore the pawn
    if (m.is_promotion())
    {
        bitboards[piece_on_to] &= ~to_bit;
        int pawn = (side_to_move == 0) ? WHITE_PAWN : BLACK_PAWN;
        bitboards[pawn] |= to_bit;
        piece_on_to = pawn;
    }

    // Move piece back
    bitboards[piece_on_to] &= ~to_bit;
    bitboards[piece_on_to] |= from_bit;

    // Handle castling: move rook back
    if (flag == KING_CASTLE)
    {
        if (side_to_move == 0)
        {
            bitboards[WHITE_ROOK] &= ~(1ULL << 5);
            bitboards[WHITE_ROOK] |= (1ULL << 7);
        }
        else
        {
            bitboards[BLACK_ROOK] &= ~(1ULL << 61);
            bitboards[BLACK_ROOK] |= (1ULL << 63);
        }
    }
    else if (flag == QUEEN_CASTLE)
    {
        if (side_to_move == 0)
        {
            bitboards[WHITE_ROOK] &= ~(1ULL << 3);
            bitboards[WHITE_ROOK] |= (1ULL << 0);
        }
        else
        {
            bitboards[BLACK_ROOK] &= ~(1ULL << 59);
            bitboards[BLACK_ROOK] |= (1ULL << 56);
        }
    }

    // Restore captured piece
    if (info.captured_piece >= 0)
    {
        if (info.was_ep_capture)
        {
            int captured_sq = (side_to_move == 0) ? to - 8 : to + 8;
            bitboards[info.captured_piece] |= (1ULL << captured_sq);
        }
        else
        {
            bitboards[info.captured_piece] |= to_bit;
        }
    }

    // Restore en passant square and castling rights
    en_passant_sq = info.prev_ep_square;
    for (int i = 0; i < 4; i++)
        castle_rights[i] = info.prev_castle[i];

    // Recompute aggregate bitboards
    bitboards[ALL_WHITE] = 0;
    bitboards[ALL_BLACK] = 0;
    for (int p = WHITE_PAWN; p <= WHITE_KING; p++)
        bitboards[ALL_WHITE] |= bitboards[p];
    for (int p = BLACK_PAWN; p <= BLACK_KING; p++)
        bitboards[ALL_BLACK] |= bitboards[p];
    bitboards[ALL_PIECES] = bitboards[ALL_WHITE] | bitboards[ALL_BLACK];
}

// ============================================================================
// CHECK DETECTION
// ============================================================================

bool Board::is_square_attacked(int sq, int by_side) const
{
    U64 square_bit = 1ULL << sq;

    // Pawn attacks
    if (by_side == 0) // White attacks
    {
        // White pawns attack diagonally upward
        // To attack sq, a white pawn would be below-left or below-right of sq
        U64 pawn_attacks = (square_bit >> 7) & NOT_A_FILE | // pawn at sq-7 attacks up-left to sq
                           (square_bit >> 9) & NOT_H_FILE;  // pawn at sq-9 attacks up-right to sq
        if (pawn_attacks & bitboards[WHITE_PAWN])
            return true;
    }
    else // Black attacks
    {
        // Black pawns attack diagonally downward
        U64 pawn_attacks = (square_bit << 7) & NOT_H_FILE | // pawn at sq+7 attacks down-right to sq
                           (square_bit << 9) & NOT_A_FILE;  // pawn at sq+9 attacks down-left to sq
        if (pawn_attacks & bitboards[BLACK_PAWN])
            return true;
    }

    // Knight attacks
    if (knight_masks[sq] & bitboards[by_side == 0 ? WHITE_KNIGHT : BLACK_KNIGHT])
        return true;

    // King attacks
    if (king_masks[sq] & bitboards[by_side == 0 ? WHITE_KING : BLACK_KING])
        return true;

    // Bishop/Queen attacks (diagonals)
    U64 bishop_rays = get_bishop_attacks(sq, bitboards[ALL_PIECES]);
    if (bishop_rays & (bitboards[by_side == 0 ? WHITE_BISHOP : BLACK_BISHOP] |
                       bitboards[by_side == 0 ? WHITE_QUEEN : BLACK_QUEEN]))
        return true;

    // Rook/Queen attacks (straights)
    U64 rook_rays = get_rook_attacks(sq, bitboards[ALL_PIECES]);
    if (rook_rays & (bitboards[by_side == 0 ? WHITE_ROOK : BLACK_ROOK] |
                     bitboards[by_side == 0 ? WHITE_QUEEN : BLACK_QUEEN]))
        return true;

    return false;
}

bool Board::is_in_check(int color) const
{
    U64 king_bb = bitboards[color == 0 ? WHITE_KING : BLACK_KING];
    if (!king_bb)
        return false;

    int king_sq = __builtin_ctzll(king_bb);
    return is_square_attacked(king_sq, 1 - color);
}

// Generate pseudo-random 64-bit numbers
U64 random_u64(std::mt19937_64 &rng)
{
    std::uniform_int_distribution<U64> dist;
    return dist(rng);
}

void Board::init_zobrist()
{
    std::mt19937_64 rng(12345); // Fixed seed for reproducibility

    // Piece-square keys
    for (int piece = 0; piece < 12; piece++)
    {
        for (int sq = 0; sq < 64; sq++)
        {
            zobrist_piece[piece][sq] = random_u64(rng);
        }
    }

    // Side to move
    zobrist_side = random_u64(rng);

    // Castling rights (16 combinations of 4 bits)
    for (int i = 0; i < 16; i++)
    {
        zobrist_castle[i] = random_u64(rng);
    }

    // En passant files
    for (int f = 0; f < 8; f++)
    {
        zobrist_ep[f] = random_u64(rng);
    }
}

void Board::compute_hash()
{
    hash = 0;

    // XOR in all pieces
    for (int piece = 0; piece < 12; piece++)
    {
        U64 bb = bitboards[piece];
        while (bb)
        {
            int sq = pop_lsb(bb);
            hash ^= zobrist_piece[piece][sq];
        }
    }

    // XOR in side to move (if black)
    if (side_to_move == 1)
        hash ^= zobrist_side;

    // XOR in castling rights
    int castle_key = (castle_rights[0] << 0) |
                     (castle_rights[1] << 1) |
                     (castle_rights[2] << 2) |
                     (castle_rights[3] << 3);
    hash ^= zobrist_castle[castle_key];

    // XOR in en passant
    if (en_passant_sq >= 0)
    {
        int ep_file = en_passant_sq % 8;
        hash ^= zobrist_ep[ep_file];
    }
}

// Push position to repetition table
void Board::push_rep()
{
    rep_table[++rep_index] = hash;
}

// Pop position from repetition table
void Board::pop_rep()
{
    rep_index--;
}

// Count how many times current position appears in search path
int Board::count_reps() const
{
    int count = 0;

    // Only check every 2 ply (positions where same side to move)
    // A repetition can only occur when it's the same side's turn
    for (int i = rep_index - 2; i >= 0; i -= 2)
    {
        if (rep_table[i] == hash)
            count++;
    }

    return count;
}