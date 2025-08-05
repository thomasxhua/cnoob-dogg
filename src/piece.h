#ifndef PIECE_H
#define PIECE_H

typedef enum
{
    PIECE_NONE,
    PIECE_WP,PIECE_WN,PIECE_WB,PIECE_WR,PIECE_WQ,PIECE_WK,
    PIECE_BP,PIECE_BN,PIECE_BB,PIECE_BR,PIECE_BQ,PIECE_BK
} piece_t;

char piece_to_char(piece_t p);

#endif // PIECE_H
