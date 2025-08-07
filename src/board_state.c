#include "board_state.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

square_t board_state_get_pseudo_legal_moves_pawns(BoardState* state, bool is_white)
{
    assert(state != NULL);
    // TODO: en passant
    const Bitboard* board = &state->board;
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t opponent_pieces = all_pieces & (is_white ? ~board->w : board->w);
    const square_t own_pawns       = board->p & (is_white ? board->w : ~board->w);
    const square_t moves           = is_white
        ? ~all_pieces & ((own_pawns << RANK_SIZE) | (own_pawns & RANK_2) << 2 * RANK_SIZE)
        : ~all_pieces & ((own_pawns >> RANK_SIZE) | (own_pawns & RANK_7) >> 2 * RANK_SIZE);
    const square_t pawns_a         = own_pawns & FILE_A;
    const square_t pawns_h         = own_pawns & FILE_H;
    const square_t pawns_inner     = own_pawns & (BOARD_FULL ^ (FILE_A|FILE_B));
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
    return moves | attacks | state->en_passant_square;
}

square_t board_state_get_pseudo_legal_moves_knights(BoardState* state, bool is_white)
{
    assert(state != NULL);
    const Bitboard* board = &state->board;
    const square_t own_knights = board->n & (is_white ? board->w : ~board->w);
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

square_t get_pseudo_legal_moves_bishops(BoardState* state, bool is_white, square_t piece)
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

square_t board_state_get_pseudo_legal_moves_bishops(BoardState* state, bool is_white)
{
    assert(state != NULL);
    return get_pseudo_legal_moves_bishops(state, is_white, state->board.b);
}

square_t get_pseudo_legal_moves_rooks(BoardState* state, bool is_white, square_t piece)
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

square_t board_state_get_pseudo_legal_moves_rooks(BoardState* state, bool is_white)
{
    assert(state != NULL);
    return get_pseudo_legal_moves_rooks(state, is_white, state->board.r);
}

square_t board_state_get_pseudo_legal_moves_queens(BoardState* state, bool is_white)
{
    assert(state != NULL);
    return
        get_pseudo_legal_moves_bishops(state, is_white, state->board.q)
        | get_pseudo_legal_moves_rooks(state, is_white, state->board.q);
}

square_t board_state_get_pseudo_legal_moves(BoardState* state, square_t from)
{
    assert(state != NULL);
    Bitboard* board = &state->board;
    const square_t* from_piece_ptr = bitboard_get_piece_ptr(board, from);
    if (!from_piece_ptr)
        return 0;
    const bool is_from_piece_white = bitboard_is_white(board, from);
    if (from_piece_ptr == &board->p)
        return board_state_get_pseudo_legal_moves_pawns(state, is_from_piece_white);
    else if (from_piece_ptr == &board->n)
        return board_state_get_pseudo_legal_moves_knights(state, is_from_piece_white);
    else if (from_piece_ptr == &board->b)
        return board_state_get_pseudo_legal_moves_bishops(state, is_from_piece_white);
    return 0;
}

