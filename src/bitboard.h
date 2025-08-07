#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#include <stdbool.h>

#include "piece.h"

typedef uint64_t square_t;

typedef enum
{
    COORD_RANK_1,
    COORD_RANK_2,
    COORD_RANK_3,
    COORD_RANK_4,
    COORD_RANK_5,
    COORD_RANK_6,
    COORD_RANK_7,
    COORD_RANK_8
} coord_rank_t;

typedef enum
{
    COORD_FILE_A,
    COORD_FILE_B,
    COORD_FILE_C,
    COORD_FILE_D,
    COORD_FILE_E,
    COORD_FILE_F,
    COORD_FILE_G,
    COORD_FILE_H
} coord_file_t;

typedef struct 
{
    square_t w,p,n,b,r,q,k;
} Bitboard;

#define RANK_SIZE 8
#define FILE_SIZE 8
#define BOARD_SIZE RANK_SIZE*FILE_SIZE

#define _SQ(x,y) static const square_t x = 1ULL << y;
_SQ(A1,000) _SQ(B1,001) _SQ(C1,002) _SQ(D1,003) _SQ(E1,004) _SQ(F1,005) _SQ(G1,006) _SQ(H1,007)
_SQ(A2,010) _SQ(B2,011) _SQ(C2,012) _SQ(D2,013) _SQ(E2,014) _SQ(F2,015) _SQ(G2,016) _SQ(H2,017)
_SQ(A3,020) _SQ(B3,021) _SQ(C3,022) _SQ(D3,023) _SQ(E3,024) _SQ(F3,025) _SQ(G3,026) _SQ(H3,027)
_SQ(A4,030) _SQ(B4,031) _SQ(C4,032) _SQ(D4,033) _SQ(E4,034) _SQ(F4,035) _SQ(G4,036) _SQ(H4,037)
_SQ(A5,040) _SQ(B5,041) _SQ(C5,042) _SQ(D5,043) _SQ(E5,044) _SQ(F5,045) _SQ(G5,046) _SQ(H5,047)
_SQ(A6,050) _SQ(B6,051) _SQ(C6,052) _SQ(D6,053) _SQ(E6,054) _SQ(F6,055) _SQ(G6,056) _SQ(H6,057)
_SQ(A7,060) _SQ(B7,061) _SQ(C7,062) _SQ(D7,063) _SQ(E7,064) _SQ(F7,065) _SQ(G7,066) _SQ(H7,067)
_SQ(A8,070) _SQ(B8,071) _SQ(C8,072) _SQ(D8,073) _SQ(E8,074) _SQ(F8,075) _SQ(G8,076) _SQ(H8,077)
#undef _SQ

static const square_t RANK_1 = A1|B1|C1|D1|E1|F1|G1|H1;
static const square_t RANK_2 = A2|B2|C2|D2|E2|F2|G2|H2;
static const square_t RANK_3 = A3|B3|C3|D3|E3|F3|G3|H3;
static const square_t RANK_4 = A4|B4|C4|D4|E4|F4|G4|H4;
static const square_t RANK_5 = A5|B5|C5|D5|E5|F5|G5|H5;
static const square_t RANK_6 = A6|B6|C6|D6|E6|F6|G6|H6;
static const square_t RANK_7 = A7|B7|C7|D7|E7|F7|G7|H7;
static const square_t RANK_8 = A8|B8|C8|D8|E8|F8|G8|H8;
static const square_t FILE_A = A1|A2|A3|A4|A5|A6|A7|A8;
static const square_t FILE_B = B1|B2|B3|B4|B5|B6|B7|B8;
static const square_t FILE_C = C1|C2|C3|C4|C5|C6|C7|C8;
static const square_t FILE_D = D1|D2|D3|D4|D5|D6|D7|D8;
static const square_t FILE_E = E1|E2|E3|E4|E5|E6|E7|E8;
static const square_t FILE_F = F1|F2|F3|F4|F5|F6|F7|F8;
static const square_t FILE_G = G1|G2|G3|G4|G5|G6|G7|G8;
static const square_t FILE_H = H1|H2|H3|H4|H5|H6|H7|H8;

static const square_t BOARD_FULL      = ~1UL;
static const square_t BOARD_QUEENSIDE = FILE_A|FILE_B|FILE_C|FILE_D;
static const square_t BOARD_KINGSIDE  = FILE_E|FILE_F|FILE_G|FILE_H;
static const square_t BOARD_EDGE      = RANK_1|RANK_8|FILE_A|FILE_H;

char* square_to_string(square_t square);

char* bitboard_to_string_annotated(const Bitboard* board, square_t annotation);
char* bitboard_to_string(const Bitboard* board);
piece_t bitboard_get_piece(const Bitboard* board, square_t square);
square_t bitboard_get_all_pieces(const Bitboard* board);

square_t* bitboard_get_piece_ptr(Bitboard* board, square_t square);
bool bitboard_is_white(Bitboard* board, square_t square);
void bitboard_set_starting_position(Bitboard* board);
piece_t bitboard_move(Bitboard* board, square_t from, square_t to);

#endif // BITBOARD_H
