#include "bitboard.h"

#include "piece.h"

#include <assert.h>
#include <stdio.h>

#define IF_PIECE_EXISTS(piece, white, black) \
    if (piece & mask) \
    { \
        p = (board->w & mask) ? white : black; \
    }

char* bitboard_to_string(const Bitboard* board)
{
    assert(board != NULL);
    static char str[BOARD_SIZE*2 + RANK_SIZE + 1] = {0};
    uint64_t file_num = FILE_SIZE;
    uint64_t mask     = 1ULL << (file_num * RANK_SIZE);
    size_t idx        = 0;
    for (uint64_t i=0; i<BOARD_SIZE; ++i)
    {
        if (i % RANK_SIZE == 0)
        {
            if (i > 0)
                str[idx++] = '\n';
            mask = 1ULL << (--file_num * RANK_SIZE);
        }
        piece p = piece_none;
        IF_PIECE_EXISTS(board->k, piece_wK, piece_bK)
        else IF_PIECE_EXISTS(board->q, piece_wQ, piece_bQ)
        else IF_PIECE_EXISTS(board->r, piece_wR, piece_bR)
        else IF_PIECE_EXISTS(board->b, piece_wB, piece_bB)
        else IF_PIECE_EXISTS(board->n, piece_wN, piece_bN)
        else IF_PIECE_EXISTS(board->p, piece_wP, piece_bP)
        str[idx++] = piece_to_char(p);
        str[idx++] = '.';
        mask <<= 1;
    }
    return str;
}

void bitboard_set_starting_position(Bitboard* board)
{
    assert(board != NULL);
    board->w |= RANK_1 | RANK_2;
    board->p |= RANK_2 | RANK_7;
    board->n |= B1|G1  | B8|G8;
    board->b |= C1|F1  | C8|F8;
    board->r |= A1|H1  | A8|H8;
    board->q |= D1     | D8;
    board->k |= E1     | E8;
}

