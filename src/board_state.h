#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include "bitboard.h"

typedef enum
{
    COLOR_WHITE,
    COLOR_BLACK
} color_t;

// "16.1.3.1: Piece placement data" ff., in: Steven J. Edwards, _Standard: Portable Game Notation Specification and Implementation Guide_, revised: 1994
// https://ia802908.us.archive.org/26/items/pgn-standard-1994-03-12/PGN_standard_1994-03-12.txt
typedef struct
{
    Bitboard board;
    uint64_t fullmove_count;
    uint64_t halfmove_clock;
    square_t en_passant_square;
    uint8_t castling_availability;
    color_t active_color;
} BoardState;

/*
- 4 move from
- 4 move to
- 2 
*/

typedef struct
{
    piece_t piece;
    square_t from,to;
    uint64_t queening_choice;
} Move;

static const uint64_t BOARD_STATE_FIELDS_CASTLING_WK  = 1ULL << 0;
static const uint64_t BOARD_STATE_FIELDS_CASTLING_WQ  = 1ULL << 1;
static const uint64_t BOARD_STATE_FIELDS_CASTLING_BK  = 1ULL << 2;
static const uint64_t BOARD_STATE_FIELDS_CASTLING_BQ  = 1ULL << 3;
static const uint64_t BOARD_STATE_FIELDS_ACTIVE_COLOR = 1ULL << 4;

square_t board_state_get_pseudo_legal_moves_pawns(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_knights(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_bishops(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_rooks(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_queens(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves(BoardState* state, square_t from);

#endif // BOARD_STATE_H

