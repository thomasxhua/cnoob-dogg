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
        G1,F3, G8,F6,
        F3,E5, B8,C6,
        F1,B5, C8,D7,
        B2,B3, A8,B8,
        C1,B2, H7,H6,
        D1,E2, G7,G5,
        H2,H3, A7,A6,
    };
    const size_t moves_size = sizeof(moves) / sizeof(moves[0]);
    
    BoardState state = {0};
    Bitboard* board = &state.board;
    state.fields |=
        BOARD_STATE_FIELDS_CASTLING_WK
        | BOARD_STATE_FIELDS_CASTLING_WQ
        | BOARD_STATE_FIELDS_CASTLING_BK;
    state.fields |= BOARD_STATE_FIELDS_ACTIVE_COLOR_W;
    state.fullmove_count = 1 + (moves_size >> 2);
    state.halfmove_clock = 0;

    bitboard_set_starting_position(board);
    for (size_t i=0; i<moves_size; ++i)
    {
        const bool is_white   = i%4==0;
        const uint64_t from   = moves[i];
        const uint64_t to     = moves[++i];
        const piece_t removed = bitboard_move(board,from,to);
        const square_t annotation = board_state_get_pseudo_legal_squares_kings(&state, !is_white, 0);
        
        char str_bitboard[BITBOARD_TO_STRING_SIZE];
        char str_from[SQUARE_TO_STRING_SIZE];
        char str_to[SQUARE_TO_STRING_SIZE];
        char str_fen[BOARD_STATE_TO_FEN_STRING_SIZE];
        bitboard_to_string_annotated(board, annotation, str_bitboard, BITBOARD_TO_STRING_SIZE);
        square_to_string(from, str_from, SQUARE_TO_STRING_SIZE);
        square_to_string(to, str_to, SQUARE_TO_STRING_SIZE);
        board_state_to_fen_string(&state, str_fen, BOARD_STATE_TO_FEN_STRING_SIZE);
        printf("\n%s\n%s-%s, removed: [%c]\nFEN: \'%s\'\n",
            str_bitboard,
            str_from,
            str_to,
            piece_to_char(removed),
            str_fen);
        Move moves[BOARD_STATE_MOVES_PIECES_SIZE];
        const size_t length = board_state_get_pseudo_legal_moves_pawns(&state, is_white, moves, BOARD_STATE_MOVES_PIECES_SIZE);
        for (size_t i=0; i<length; ++i)
        {
            char str_from_moves[SQUARE_TO_STRING_SIZE];
            char str_to_moves[SQUARE_TO_STRING_SIZE];
            square_to_string(moves[i].from, str_from_moves, SQUARE_TO_STRING_SIZE);
            square_to_string(moves[i].to, str_to_moves, SQUARE_TO_STRING_SIZE);
            printf("%s, ", str_to_moves);
        }
        printf("\n");
    }

    return 0;
}

