#include "board_state.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

square_t get_pseudo_legal_moves_pawns(BoardState* state, bool is_white)
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

square_t board_state_get_pseudo_legal_moves(BoardState* state, square_t from)
{
    assert(state != NULL);
    Bitboard* board = &state->board;
    const square_t* from_piece_ptr = bitboard_get_piece_ptr(board, from);
    if (!from_piece_ptr)
        return 0;
    const bool is_from_piece_white = bitboard_is_white(board, from);
    //size_t count = 0;
    if (from_piece_ptr == &board->p)
    {
        return is_from_piece_white
            ? get_pseudo_legal_moves_pawns(state, true)
            : get_pseudo_legal_moves_pawns(state, false);
    }
    return 0;
}

