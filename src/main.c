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
    board_state_set_fen_string(&state, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", BOARD_STATE_TO_FEN_STRING_SIZE);
    Move moves[BOARD_STATE_MOVES_SIZE];
    const size_t count = board_state_get_legal_moves(&state, true, moves, BOARD_STATE_MOVES_SIZE);
    for (size_t i=0; i<count; ++i)
    {
        char move_str[BOARD_STATE_MOVE_TO_STRING_SIZE];
        board_state_move_to_string(&state, &moves[i], move_str, BOARD_STATE_MOVE_TO_STRING_SIZE);
        printf("%s, ", move_str);
    }
    printf("\n");
    board_state_print(&state, board_state_get_pseudo_legal_squares_kings(&state, true, BOARD_FULL));
}

#define PRINT_MANUAL() printf("Usage:\n%s perft <depth> moves...\n%s fen moves...\n%s perft-fen \"FEN\"", argv[0], argv[0], argv[0])

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
    else if (strcmp(argv[1], "perft-fen") == 0)
    {
        const int size = 4;
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
            printf("moves_size: %llu\n", moves_size);
            BoardState state = {0};
            board_state_set_fen_string(&state, argv[3], BOARD_STATE_SET_FEN_STRING_SIZE);
            for (size_t i=0; i<moves_size; ++i)
                board_state_apply_move(&state, &moves[i]);
            free(moves);
            char fen_str[BOARD_STATE_SET_FEN_STRING_SIZE];
            board_state_to_fen_string(&state, fen_str, BOARD_STATE_TO_FEN_STRING_SIZE);
            perft_test_fen(depth, fen_str, BOARD_STATE_TO_FEN_STRING_SIZE, PERFT_TEST_VERBOSE_ALL);
        }
        else
        {
            perft_test_fen(depth, argv[3], BOARD_STATE_TO_FEN_STRING_SIZE, PERFT_TEST_VERBOSE_ALL);
        }
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
        free(moves);
    }
    return 0;
}

