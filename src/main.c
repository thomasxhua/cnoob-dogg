#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "bitboard.h"
#include "board_state.h"
#include "perft.h"

void main_no_args()
{
    BoardState state = {0};
    board_state_init(&state);
#if 1
    board_state_apply_move(&state, &(Move){C2,C3,0});
    board_state_apply_move(&state, &(Move){F7,F5,0});
    board_state_apply_move(&state, &(Move){A2,A3,0});
    board_state_apply_move(&state, &(Move){F5,F4,0});
    board_state_apply_move(&state, &(Move){E2,E4,0});
    board_state_apply_move(&state, &(Move){F4,E3,0});
    board_state_print(&state, 0);
    printf("\n");
    board_state_set_fen_string(&state, "rnbqkbnr/ppppp1pp/8/8/8/P1P1p3/1P1P1PPP/RNBQKBNR w KQkq - 0 3", BOARD_STATE_SET_FEN_STRING_SIZE);
    board_state_print(&state, 0);
#else
    board_state_print(&state, 0);
    bitboard_clear_square(&state.board, A1);
    board_state_print(&state, 0);
#endif
}

#define PRINT_MANUAL() printf("Usage:\n%s perft <depth> [moves...]\n%s fen [moves...]", argv[0], argv[0])

#define STRING_TO_MOVE_SIZE 4

Move string_to_move(const char* str, size_t str_size)
{
    assert(str_size >= STRING_TO_MOVE_SIZE);
    const square_t from = string_to_square(str, STRING_TO_SQUARE_SIZE);
    const square_t to   = string_to_square(str + STRING_TO_SQUARE_SIZE, STRING_TO_SQUARE_SIZE);
    return (Move)
    {
        .from   = from,
        .to     = to,
        .fields = 0
    };
} 

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        main_no_args();
        return 0;
    };

    if (strcmp(argv[1], "perft") == 0)
    {
        const int size = 3;
        if (argc < size)
        {
            PRINT_MANUAL();
            return 1;
        }
        const uint64_t depth = atoi(argv[2]);
        if (depth <= 0)
        {
            PRINT_MANUAL();
            return 1;
        }
        Move* moves = NULL;
        const size_t moves_size = argc - size;
        if (argc > size)
        {
            moves = malloc(moves_size * sizeof(Move)); // TODO failure
            const char** move_strs = (const char**)&argv[size];
            for (size_t i=0; i<moves_size; ++i)
            {
                const char* move_str = move_strs[i];
                moves[i] = string_to_move(move_str, strlen(move_str));
            }
        }
        perft_test(depth, moves, moves_size, PERFT_TEST_VERBOSE_ALL);
        free(moves);
    }
    else if (strcmp(argv[1], "fen") == 0)
    {
        const int size = 2;
        if (argc < size)
        {
            PRINT_MANUAL();
            return 1;
        }
        Move* moves = NULL;
        const size_t moves_size = argc - size;
        if (argc > size)
        {
            moves = malloc(moves_size * sizeof(Move)); // TODO failure
            const char** move_strs = (const char**)&argv[size];
            for (size_t i=0; i<moves_size; ++i)
            {
                const char* move_str = move_strs[i];
                moves[i] = string_to_move(move_str, strlen(move_str));
            }
        }
        BoardState state = {0};
        board_state_init(&state);
        for (size_t i=0; i<moves_size; ++i)
            board_state_apply_move(&state, &moves[i]);
        board_state_print(&state, 0);
        free(moves);
    }
    return 0;
}

