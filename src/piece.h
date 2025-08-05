#ifndef PIECE_H
#define PIECE_H

typedef enum
{
    piece_none,
    piece_wP,piece_wN,piece_wB,piece_wR,piece_wQ,piece_wK,
    piece_bP,piece_bN,piece_bB,piece_bR,piece_bQ,piece_bK
} piece;

char piece_to_char(piece p);

#endif // PIECE_H
