#include <stdio.h>

#include "bitboard.h"

int main()
{
    Bitboard board = {0};
    bitboard_set_starting_position(&board);
    printf(bitboard_to_string(&board));
    return 0;
}

