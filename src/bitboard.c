#include "bitboard.h"

#include "piece.h"
#include "utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

char* square_to_string(square_t square)
{
    switch (square)
    {
        #define _SQ(x,y) case 1ULL << y: return x;
        _SQ("a1",000) _SQ("b1",001) _SQ("c1",002) _SQ("d1",003) _SQ("e1",004) _SQ("f1",005) _SQ("g1",006) _SQ("h1",007)
        _SQ("a2",010) _SQ("b2",011) _SQ("c2",012) _SQ("d2",013) _SQ("e2",014) _SQ("f2",015) _SQ("g2",016) _SQ("h2",017)
        _SQ("a3",020) _SQ("b3",021) _SQ("c3",022) _SQ("d3",023) _SQ("e3",024) _SQ("f3",025) _SQ("g3",026) _SQ("h3",027)
        _SQ("a4",030) _SQ("b4",031) _SQ("c4",032) _SQ("d4",033) _SQ("e4",034) _SQ("f4",035) _SQ("g4",036) _SQ("h4",037)
        _SQ("a5",040) _SQ("b5",041) _SQ("c5",042) _SQ("d5",043) _SQ("e5",044) _SQ("f5",045) _SQ("g5",046) _SQ("h5",047)
        _SQ("a6",050) _SQ("b6",051) _SQ("c6",052) _SQ("d6",053) _SQ("e6",054) _SQ("f6",055) _SQ("g6",056) _SQ("h6",057)
        _SQ("a7",060) _SQ("b7",061) _SQ("c7",062) _SQ("d7",063) _SQ("e7",064) _SQ("f7",065) _SQ("g7",066) _SQ("h7",067)
        _SQ("a8",070) _SQ("b8",071) _SQ("c8",072) _SQ("d8",073) _SQ("e8",074) _SQ("f8",075) _SQ("g8",076) _SQ("h8",077)
        #undef _SQ
        default: return "??";
    }
}

char* bitboard_to_string_annotated(const Bitboard* board, square_t annotation)
{
    assert(board != NULL);
    static char str[BOARD_SIZE*2 + RANK_SIZE + 1] = {0};
    uint64_t file_num = FILE_SIZE;
    square_t square   = 1ULL << (file_num * RANK_SIZE);
    size_t idx        = 0;
    for (uint64_t i=0; i<BOARD_SIZE; ++i)
    {
        if (i % RANK_SIZE == 0)
        {
            if (i > 0)
                str[idx++] = '\n';
            square = 1ULL << (--file_num * RANK_SIZE);
        }
        str[idx++] = piece_to_char(bitboard_get_piece(board, square));
        str[idx++] = (square & annotation) ? '*' : '.';
        square <<= 1;
    }
    return str;
}

char* bitboard_to_string(const Bitboard* board)
{
    return bitboard_to_string_annotated(board, 0);
}

piece_t bitboard_get_piece(const Bitboard* board, square_t square)
{
    assert(board != NULL);
    if (board->k & square)
        return (board->w & square) ? PIECE_WK : PIECE_BK;
    else if (board->q & square)
        return (board->w & square) ? PIECE_WQ : PIECE_BQ;
    else if (board->r & square)
        return (board->w & square) ? PIECE_WR : PIECE_BR;
    else if (board->b & square)
        return (board->w & square) ? PIECE_WB : PIECE_BB;
    else if (board->n & square)
        return (board->w & square) ? PIECE_WN : PIECE_BN;
    else if (board->p & square)
        return (board->w & square) ? PIECE_WP : PIECE_BP;
    return PIECE_NONE;
}

square_t bitboard_get_all_pieces(const Bitboard* board)
{
    assert(board != NULL);
    return board->p|board->n|board->b|board->r|board->q|board->k;
}

uint64_t* bitboard_get_piece_ptr(Bitboard* board, square_t square)
{
    assert(board != NULL);
    if (board->k & square) return &board->k;
    if (board->q & square) return &board->q;
    if (board->r & square) return &board->r;
    if (board->b & square) return &board->b;
    if (board->n & square) return &board->n;
    if (board->p & square) return &board->p;
    return NULL;
}

bool bitboard_is_white(Bitboard* board, square_t square)
{
    assert(board != NULL);
    return board->w & square;
}

void bitboard_set_starting_position(Bitboard* board)
{
    assert(board != NULL);
    board->w = RANK_1 | RANK_2;
    board->p = RANK_2 | RANK_7;
    board->n = B1|G1  | B8|G8;
    board->b = C1|F1  | C8|F8;
    board->r = A1|H1  | A8|H8;
    board->q = D1     | D8;
    board->k = E1     | E8;
}

piece_t bitboard_move(Bitboard* board, square_t from, square_t to)
{
    assert(board != NULL);
    assert(is_power_of_two(from) && is_power_of_two(to));
    square_t* from_piece_ptr = bitboard_get_piece_ptr(board, from);
    if (!from_piece_ptr)
        return PIECE_NONE;
    const piece_t to_piece = bitboard_get_piece(board, to);
    square_t* to_piece_ptr = bitboard_get_piece_ptr(board, to);
    bool is_from_piece_white = false;
    // remove from_piece
    (*from_piece_ptr) &= ~from; // TODO: maybe bitminus, a & ~b
    if (board->w & from)
    {
        board->w &= ~from;
        is_from_piece_white = true;
    }
    if (to_piece_ptr)
    {
        // remove to_piece
        (*to_piece_ptr) &= ~to;
        if (board->w & to)
            board->w &= ~to;
    }
    // add from_piece
    (*from_piece_ptr) |= to;
    if (is_from_piece_white)
        board->w |= to;
    return to_piece;
}

