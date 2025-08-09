#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include "bitboard.h"

typedef enum
{
    COLOR_WHITE,
    COLOR_BLACK
} color_t;

// "16.1 FEN", Edwards, _Standard: Portable Game Notation Specification and Implementation Guide_
// https://ia802908.us.archive.org/26/items/pgn-standard-1994-03-12/PGN_standard_1994-03-12.txt

typedef struct
{
    Bitboard board;
    uint64_t fullmove_count;
    uint64_t halfmove_clock;
    square_t en_passant_square;
    uint8_t fields;
} BoardState;

void board_state_to_fen_string(const BoardState* state, char* str, size_t str_size);

typedef struct
{
    piece_t piece;
    square_t from,to;
    piece_t removed_piece;
    uint8_t queening;
} Move;

#define BOARD_STATE_TO_FEN_STRING_SIZE \
    RANK_SIZE*FILE_SIZE + (FILE_SIZE-1) /* piece placement data */ \
    + 2  /* active color */ \
    + 5  /* castling */ \
    + 3  /* en passant */ \
    + 4  /* halfmove clock */ \
    + 21 /* fullmove count */

static const uint8_t BOARD_STATE_FIELDS_CASTLING_WK    = 1ULL << 0;
static const uint8_t BOARD_STATE_FIELDS_CASTLING_WQ    = 1ULL << 1;
static const uint8_t BOARD_STATE_FIELDS_CASTLING_BK    = 1ULL << 2;
static const uint8_t BOARD_STATE_FIELDS_CASTLING_BQ    = 1ULL << 3;
static const uint8_t BOARD_STATE_FIELDS_CASTLING       = 0xFULL;
static const uint8_t BOARD_STATE_FIELDS_ACTIVE_COLOR_W = 1ULL << 4;

static const uint8_t MOVE_QUEENING_CHOICE_N = 1ULL << 0;
static const uint8_t MOVE_QUEENING_CHOICE_B = 1ULL << 1;
static const uint8_t MOVE_QUEENING_CHOICE_R = 1ULL << 2;
static const uint8_t MOVE_QUEENING_CHOICE_Q = 1ULL << 3;

square_t board_state_get_pseudo_legal_moves_pawns(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_knights(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_bishops(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_rooks(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_queens(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves_kings(BoardState* state, bool is_white);
square_t board_state_get_pseudo_legal_moves(BoardState* state, square_t from);

#endif // BOARD_STATE_H

