#include "piece.h"

char piece_to_char(piece p)
{
    switch (p)
    {
        case piece_wP: return 'P';
        case piece_wN: return 'N';
        case piece_wB: return 'B';
        case piece_wR: return 'R';
        case piece_wQ: return 'Q';
        case piece_wK: return 'K';
        case piece_bP: return 'p';
        case piece_bN: return 'n';
        case piece_bB: return 'b';
        case piece_bR: return 'r';
        case piece_bQ: return 'q';
        case piece_bK: return 'k';
        default:       return ' ';
    }
}

