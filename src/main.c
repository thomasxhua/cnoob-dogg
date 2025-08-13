#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "bitboard.h"
#include "board_state.h"
#include "perft.h"

#define CNOOBDOGG_MANUAL \
    "Usage:\n" \
    "  %s perft <depth> moves...    Run perft depth after applying moves to starting position.\n" \
    "  %s fen moves...              Generate fen string after applying moves to starting position.\n" \
    "  %s perft-fen <fen> moves...  Run perft depth after applying moves to fen position.\n"

#define CNOOBDOGG_PRINT_MANUAL() printf(CNOOBDOGG_MANUAL, argv[0], argv[0], argv[0])

int main_no_args();
int main_perft(int argc, char* argv[]);
int main_perft_fen(int argc, char* argv[]);
int main_fen(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    CNOOBDOGG_PRINT_MANUAL();
    if (argc < 2)
        return main_no_args();
    else if (strcmp(argv[1], "perft") == 0)
        return main_perft(argc, argv);
    else if (strcmp(argv[1], "perft-fen") == 0)
        return main_perft_fen(argc, argv);
    else if (strcmp(argv[1], "fen") == 0)
        return main_fen(argc, argv);
    return 0;
}

int main_no_args()
{
    return 0;
}

int main_perft(int argc, char* argv[])
{
    const int size = 3;
    if (argc < size)
    {
        CNOOBDOGG_PRINT_MANUAL();
        return 1;
    }
    const uint64_t depth = atoi(argv[2]);
    if (depth <= 0)
    {
        CNOOBDOGG_PRINT_MANUAL();
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
    return 0;
}

int main_perft_fen(int argc, char* argv[])
{

    const int size = 4;
    if (argc < size)
    {
        CNOOBDOGG_PRINT_MANUAL();
        return 1;
    }
    const uint64_t depth = atoi(argv[2]);
    if (depth <= 0)
    {
        CNOOBDOGG_PRINT_MANUAL();
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
    return 0;
}

int main_fen(int argc, char* argv[])
{
    const int size = 2;
    if (argc < size)
    {
        CNOOBDOGG_PRINT_MANUAL();
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
    return 0;
}

