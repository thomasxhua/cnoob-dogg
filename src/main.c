#include <stdio.h>
#include <stdlib.h>

#include "board_state.h"

int main()
{
    const uint64_t moves[] =
    {
        E2,E4, D7,D5,
        E4,D5, D8,D5,
        B1,C3, D5,A5,
    };
    const size_t moves_size = sizeof(moves) / sizeof(moves[0]);
    
    BoardState state = {0};
    Bitboard* board = &state.board;
    bitboard_set_starting_position(board);

    for (size_t i=0; i<moves_size; ++i)
    {
        const square_t color  = ((i%4)==0)?A7:A2;
        const uint64_t from   = moves[i];
        const uint64_t to     = moves[++i];
        const piece_t removed = bitboard_move(board,from,to);
        const square_t annotation = board_state_get_pseudo_legal_moves(&state, color);
        printf("\n%s\nremoved: %c (%s,%s)\n",
            bitboard_to_string_annotated(board, annotation),
            piece_to_char(removed),
            square_to_string(from),
            square_to_string(to));
    }

    return 0;
}

