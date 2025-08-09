#ifndef BOARD_STATE_H
#define BOARD_STATE_H

#include "bitboard.h"

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
    square_t from,to;
    piece_t from_piece,to_piece;
    uint8_t fields;
    bool is_white_to_move;
} Move;

#define BOARD_STATE_TO_FEN_STRING_SIZE \
    BOARD_SIZE + (FILE_SIZE-1) /* board */ \
    + 2  /* active color */ \
    + 5  /* castling */ \
    + 3  /* en passant */ \
    + 4  /* halfmove clock */ \
    + 22 /* fullmove count */
#define BOARD_STATE_MOVES_PIECES_SIZE 64
#define BOARD_STATE_MOVES_SIZE 256

static const uint8_t BOARD_STATE_FIELDS_CASTLING_WK    = 1ULL << 0;
static const uint8_t BOARD_STATE_FIELDS_CASTLING_WQ    = 1ULL << 1;
static const uint8_t BOARD_STATE_FIELDS_CASTLING_BK    = 1ULL << 2;
static const uint8_t BOARD_STATE_FIELDS_CASTLING_BQ    = 1ULL << 3;
static const uint8_t BOARD_STATE_FIELDS_CASTLING       = 0xFULL;
static const uint8_t BOARD_STATE_FIELDS_ACTIVE_COLOR_W = 1ULL << 4;

static const uint8_t MOVE_FIELDS_QUEENING_CHOICE_N = 1ULL << 0;
static const uint8_t MOVE_FIELDS_QUEENING_CHOICE_B = 1ULL << 1;
static const uint8_t MOVE_FIELDS_QUEENING_CHOICE_R = 1ULL << 2;
static const uint8_t MOVE_FIELDS_QUEENING_CHOICE_Q = 1ULL << 3;
static const uint8_t MOVE_FIELDS_ACTIVE_COLOR_W    = 1ULL << 4;

square_t board_state_get_pseudo_legal_squares_pawns(const BoardState* state, bool is_white, square_t selection);
square_t board_state_get_pseudo_legal_squares_knights(const BoardState* state, bool is_white, square_t selection);
square_t board_state_get_pseudo_legal_squares_bishops(const BoardState* state, bool is_white, square_t selection);
square_t board_state_get_pseudo_legal_squares_rooks(const BoardState* state, bool is_white, square_t selection);
square_t board_state_get_pseudo_legal_squares_queens(const BoardState* state, bool is_white, square_t selection);
square_t board_state_get_pseudo_legal_squares_kings(const BoardState* state, bool is_white, square_t selection);

size_t board_state_get_pseudo_legal_moves_pawns(const BoardState* state, bool is_white, Move* moves, size_t moves_size);
size_t board_state_get_pseudo_legal_moves_knights(const BoardState* state, bool is_white, Move* moves, size_t moves_size);
size_t board_state_get_pseudo_legal_moves_bishops(const BoardState* state, bool is_white, Move* moves, size_t moves_size);
size_t board_state_get_pseudo_legal_moves_rooks(const BoardState* state, bool is_white, Move* moves, size_t moves_size);
size_t board_state_get_pseudo_legal_moves_queens(const BoardState* state, bool is_white, Move* moves, size_t moves_size);
size_t board_state_get_pseudo_legal_moves_kings(const BoardState* state, bool is_white, Move* moves, size_t moves_size);

size_t board_state_get_pseudo_legal_moves(const BoardState* state, bool is_white, Move* moves, size_t moves_size);

void board_state_pseudo_apply_move(BoardState* state, const Move* moves);

#endif // BOARD_STATE_H

