#include "piece.h"

char piece_to_char(piece_t piece)
{
    switch (piece)
    {
        case PIECE_WP: return 'P';
        case PIECE_WN: return 'N';
        case PIECE_WB: return 'B';
        case PIECE_WR: return 'R';
        case PIECE_WQ: return 'Q';
        case PIECE_WK: return 'K';
        case PIECE_BP: return 'p';
        case PIECE_BN: return 'n';
        case PIECE_BB: return 'b';
        case PIECE_BR: return 'r';
        case PIECE_BQ: return 'q';
        case PIECE_BK: return 'k';
        default:       return ' ';
    }
}

