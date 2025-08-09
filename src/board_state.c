#include "board_state.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "utils.h"

void board_state_to_fen_string(const BoardState* state, char* str, size_t str_size)
{
    assert(state != NULL);
    assert(str != NULL);
    assert(str_size >= BOARD_STATE_TO_FEN_STRING_SIZE);
    const Bitboard* board = &state->board;
    // piece placement data
    uint64_t empty_counter = 0;
    uint64_t file_num = FILE_SIZE;
    square_t square   = 1ULL << (file_num * RANK_SIZE);
    size_t idx        = 0;
    for (uint64_t i=0; i<BOARD_SIZE; ++i)
    {
        if (i % RANK_SIZE == 0)
        {
            if (i > 0)
            {
                if (empty_counter)
                {
                    str[idx++] = '0' + empty_counter;
                    empty_counter = 0;
                }
                str[idx++] = '/';
            }
            square = 1ULL << (--file_num * RANK_SIZE);
        }
        const piece_t piece = bitboard_get_piece(board, square);
        if (piece != PIECE_NONE)
        {
            if (empty_counter)
            {
                str[idx++] = '0' + empty_counter;
                empty_counter = 0;
            }
            str[idx++] = piece_to_char(piece);
            empty_counter = 0;
        }
        else
        {
            ++empty_counter;
        }
        square <<= 1;
    }
    if (empty_counter)
    {
        str[idx++] = '0' + empty_counter;
        empty_counter = 0;
    }
    // active color
    str[idx++] = ' ';
    str[idx++] = (state->fields & BOARD_STATE_FIELDS_ACTIVE_COLOR_W) ? 'w' : 'b';
    // castling
    str[idx++] = ' ';
    if (state->fields & BOARD_STATE_FIELDS_CASTLING)
    {
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_WK) str[idx++] = 'K';
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_WQ) str[idx++] = 'Q';
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_BK) str[idx++] = 'k';
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_BQ) str[idx++] = 'q';
    }
    else
    {
        str[idx++] = '-';
    }
    // en passant square
    str[idx++] = ' ';
    if (state->en_passant_square)
    {
        char square_str[SQUARE_TO_STRING_SIZE];
        square_to_string(state->en_passant_square, square_str, SQUARE_TO_STRING_SIZE);
        str[idx++] = square_str[0];
        str[idx++] = square_str[1];
    }
    else
    {
        str[idx++] = '-';
    }
    // halfmove clock
    str[idx++] = ' ';
    char halfmove_str[UINT64_TO_STRING_SIZE];
    uint64_to_string(state->halfmove_clock, halfmove_str, UINT64_TO_STRING_SIZE);
    for (char* ptr = halfmove_str; *ptr != '\0'; ++ptr)
        str[idx++] = *ptr;
    // fullmove count
    str[idx++] = ' ';
    char fullmove_str[UINT64_TO_STRING_SIZE];
    uint64_to_string(state->fullmove_count, fullmove_str, UINT64_TO_STRING_SIZE);
    for (char* ptr = fullmove_str; *ptr != '\0'; ++ptr)
        str[idx++] = *ptr;
    str[idx++] = '\0';
}

square_t board_state_get_pseudo_legal_squares_pawns(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board = &state->board;
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t opponent_pieces = all_pieces & (is_white ? ~board->w : board->w);
    const square_t own_pawns       = selection & board->p & (is_white ? board->w : ~board->w);
    const square_t moves_once      = is_white
        ? ~all_pieces & (own_pawns << RANK_SIZE)
        : ~all_pieces & (own_pawns >> RANK_SIZE);
    const square_t moves_twice     = is_white
        ? ~all_pieces & ((~all_pieces & ((own_pawns & RANK_2) << RANK_SIZE)) << RANK_SIZE)
        : ~all_pieces & ((~all_pieces & ((own_pawns & RANK_7) >> RANK_SIZE)) >> RANK_SIZE);
    const square_t pawns_a         = own_pawns & FILE_A;
    const square_t pawns_h         = own_pawns & FILE_H;
    const square_t pawns_inner     = own_pawns & ~(FILE_A|FILE_H);
    const square_t attacks         = is_white
        ? opponent_pieces & (
            pawns_a       << (RANK_SIZE + 1)
            | pawns_inner << (RANK_SIZE + 1)
            | pawns_inner << (RANK_SIZE - 1)
            | pawns_h     << (RANK_SIZE - 1))
        : opponent_pieces & (
            pawns_a       >> (RANK_SIZE - 1)
            | pawns_inner >> (RANK_SIZE - 1)
            | pawns_inner >> (RANK_SIZE + 1)
            | pawns_h     >> (RANK_SIZE + 1));
    return moves_once | moves_twice | attacks | state->en_passant_square;
}

square_t board_state_get_pseudo_legal_squares_knights(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board = &state->board;
    const square_t own_knights = selection & board->n & (is_white ? board->w : ~board->w);
    const square_t own_pieces  = bitboard_get_all_pieces(board) & (is_white ? board->w : ~board->w);
    //   .NW .NE .
    //  WN . . .EN
    //   . .N. . .
    //  WS . . .ES
    //   .SW .SE .
    return ~own_pieces & (
        (own_knights   & ~(FILE_A        | RANK_8|RANK_7)) << ((2 * RANK_SIZE) - 1)   // NW
        | (own_knights & ~(FILE_H        | RANK_8|RANK_7)) << ((2 * RANK_SIZE) + 1)   // NE
        | (own_knights & ~(FILE_A|FILE_B | RANK_8       )) << ((1 * RANK_SIZE) - 2)   // WN
        | (own_knights & ~(FILE_G|FILE_H | RANK_8       )) << ((1 * RANK_SIZE) + 2)   // EN
        | (own_knights & ~(FILE_A|FILE_B | RANK_1       )) >> ((1 * RANK_SIZE) + 2)   // WS
        | (own_knights & ~(FILE_A|FILE_B | RANK_1       )) >> ((1 * RANK_SIZE) - 2)   // ES
        | (own_knights & ~(FILE_A        | RANK_1|RANK_2)) >> ((2 * RANK_SIZE) + 1)   // SW
        | (own_knights & ~(FILE_H        | RANK_1|RANK_2)) >> ((2 * RANK_SIZE) - 1)); // SE
}

square_t get_pseudo_legal_squares_bishops(const BoardState* state, bool is_white, square_t piece)
{
    assert(state != NULL);
    const Bitboard* board          = &state->board;
    const square_t own_bishops     = piece & (is_white ? board->w : ~board->w);
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t own_pieces      = all_pieces & (is_white ? board->w : ~board->w);
    const square_t opponent_pieces = all_pieces & (is_white ? ~board->w : board->w);
    square_t moves = 0;
    for (square_t square = 1ULL; square; square <<= 1)
    {
        if (!(own_bishops & square))
            continue;
        square_t temp;
        // NW
        temp = square;
        for (;;)
        {
            if (temp & (RANK_8|FILE_A)) break;
            temp <<= RANK_SIZE - 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // NE
        temp = square;
        for (;;)
        {
            if (temp & (RANK_8|FILE_H)) break;
            temp <<= RANK_SIZE + 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // SW
        temp = square;
        for (;;)
        {
            if (temp & (RANK_1|FILE_A)) break;
            temp >>= RANK_SIZE + 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // SE
        temp = square;
        for (;;)
        {
            if (temp & (RANK_1|FILE_H)) break;
            temp >>= RANK_SIZE - 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
    }
    return moves;
}

square_t board_state_get_pseudo_legal_squares_bishops(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    return get_pseudo_legal_squares_bishops(state, is_white, selection & state->board.b);
}

square_t get_pseudo_legal_squares_rooks(const BoardState* state, bool is_white, square_t piece)
{
    assert(state != NULL);
    const Bitboard* board          = &state->board;
    const square_t own_rooks       = piece & (is_white ? board->w : ~board->w);
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t own_pieces      = all_pieces & (is_white ? board->w : ~board->w);
    const square_t opponent_pieces = all_pieces & (is_white ? ~board->w : board->w);
    square_t moves = 0;
    for (square_t square = 1ULL; square; square <<= 1)
    {
        if (!(own_rooks & square))
            continue;
        square_t temp;
        // N
        temp = square;
        for (;;)
        {
            if (temp & RANK_8) break;
            temp <<= RANK_SIZE;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // E
        temp = square;
        for (;;)
        {
            if (temp & FILE_H) break;
            temp <<= 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // S
        temp = square;
        for (;;)
        {
            if (temp & RANK_1) break;
            temp >>= RANK_SIZE;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // W
        temp = square;
        for (;;)
        {
            if (temp & FILE_A) break;
            temp >>= 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
    }
    return moves;
}

square_t board_state_get_pseudo_legal_squares_rooks(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    return get_pseudo_legal_squares_rooks(state, is_white, selection & state->board.r);
}

square_t board_state_get_pseudo_legal_squares_queens(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    return
        get_pseudo_legal_squares_bishops(state, is_white, selection & state->board.q)
        | get_pseudo_legal_squares_rooks(state, is_white, selection & state->board.q);
}

square_t board_state_get_pseudo_legal_squares_kings(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board     = &state->board;
    const square_t own_kings  = selection & board->k & (is_white ? board->w : ~board->w);
    const square_t all_pieces = bitboard_get_all_pieces(board);
    const square_t own_pieces = all_pieces & (is_white ? board->w : ~board->w);
    square_t moves = 0;
    // 1.2.3.
    // 4.K.5.
    // 6.7.8.
    for (square_t square = 1ULL; square; square <<= 1)
    {
        if (!(own_kings & square))
            continue;
        moves |=
            ((square   & (RANK_8|FILE_A)) ? 0 : (square << (RANK_SIZE - 1)))  // 1
            | ((square & (RANK_8       )) ? 0 : (square << (RANK_SIZE)))      // 2
            | ((square & (RANK_8|FILE_H)) ? 0 : (square << (RANK_SIZE + 1)))  // 3
            | ((square & (       FILE_A)) ? 0 : (square >> 1))                // 4
            | ((square & (       FILE_H)) ? 0 : (square << 1))                // 5
            | ((square & (RANK_1|FILE_A)) ? 0 : (square >> (RANK_SIZE + 1)))  // 6
            | ((square & (RANK_1       )) ? 0 : (square >> (RANK_SIZE)))      // 7
            | ((square & (RANK_1|FILE_H)) ? 0 : (square >> (RANK_SIZE - 1))); // 8
    }
    moves &= ~own_pieces;
    // castling should only be available with kings on starting squares and will not be checked
    if (is_white)
    {
        moves |=
            (((state->fields   & BOARD_STATE_FIELDS_CASTLING_WK) && !(all_pieces & (F1|G1)))    ? G1 : 0)
            | (((state->fields & BOARD_STATE_FIELDS_CASTLING_WQ) && !(all_pieces & (B1|C1|D1))) ? C1 : 0);
    }
    else
    {
        moves |=
            (((state->fields   & BOARD_STATE_FIELDS_CASTLING_BK) && !(all_pieces & (F8|G8)))    ? G8 : 0)
            | (((state->fields & BOARD_STATE_FIELDS_CASTLING_BQ) && !(all_pieces & (B8|C8|D8))) ? C8 : 0);
    }
    return moves;
}

#define BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(name, lowercase, uppercase) \
    size_t board_state_get_pseudo_legal_moves_##name(const BoardState* state, bool is_white, Move* moves, size_t moves_size) \
    { \
        assert(state != NULL); \
        assert(moves != NULL); \
        assert(moves_size <= BOARD_STATE_MOVES_PIECES_SIZE); \
        const Bitboard* board     = &state->board; \
        const square_t own_##name = board->lowercase & (is_white ? board->w : ~board->w); \
        size_t idx = 0; \
        for (square_t from = 1ULL; from; from <<= 1) \
        { \
            if (!(from & own_##name)) \
                continue; \
            const square_t square_moves = board_state_get_pseudo_legal_squares_##name(state, is_white, from); \
            for (square_t to = 1ULL; to; to <<= 1) \
            { \
                if (!(to & square_moves)) \
                    continue; \
                const piece_t to_piece = bitboard_get_piece(board, to); \
                moves[idx++] = (Move) \
                { \
                    .from_piece = is_white ? PIECE_W##uppercase : PIECE_B##uppercase, \
                    .to_piece   = to_piece, \
                    .from       = from, \
                    .to         = to \
                }; \
            } \
        } \
        return idx; \
    }
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(pawns, p, P)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(knights, n, N)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(bishops, b, B)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(rooks, r, R)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(queens, q, Q)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(kings, k, K)
#undef BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE

size_t board_state_get_pseudo_legal_moves(const BoardState* state, bool is_white, Move* moves, size_t moves_size)
{
    assert(state != NULL);
    assert(moves != NULL);
    assert(moves_size <= BOARD_STATE_MOVES_SIZE);
    if (is_white)
        moves->fields |= MOVE_FIELDS_ACTIVE_COLOR_W;
    else
        moves->fields &= ~MOVE_FIELDS_ACTIVE_COLOR_W;
    size_t idx = 0;
    idx += board_state_get_pseudo_legal_moves_pawns(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_knights(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_bishops(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_rooks(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_queens(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_kings(state, is_white, moves + idx, moves_size);
    return idx;
}

void board_state_pseudo_apply_move(BoardState* state, const Move* moves)
{
    assert(state != NULL);
    assert(moves != NULL);
    // TODO:
    // - add return type for execution info
    // - handle en passant
    // - handle pawn promotion
    // - handle castling
    // - check for king attacks
}

