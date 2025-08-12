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
}

#define PRINT_MANUAL() printf("Usage:\n%s perft <depth> [moves...]\n", argv[0])

#define STRING_TO_MOVE_SIZE 4

Move string_to_move(const char* str, size_t str_size)
{
    assert(str_size >= STRING_TO_MOVE_SIZE);
    square_t from_file=0,from_rank=0,to_file=0,to_rank=0;
    switch (str[0])
    {
        case 'a': case 'A': from_file = FILE_A; break;
        case 'b': case 'B': from_file = FILE_B; break;
        case 'c': case 'C': from_file = FILE_C; break;
        case 'd': case 'D': from_file = FILE_D; break;
        case 'e': case 'E': from_file = FILE_E; break;
        case 'f': case 'F': from_file = FILE_F; break;
        case 'g': case 'G': from_file = FILE_G; break;
        case 'h': case 'H': from_file = FILE_H; break;
        default:            from_file = 0;      break;
    }
    switch (str[1])
    {
        case '1': from_rank = RANK_1; break;
        case '2': from_rank = RANK_2; break;
        case '3': from_rank = RANK_3; break;
        case '4': from_rank = RANK_4; break;
        case '5': from_rank = RANK_5; break;
        case '6': from_rank = RANK_6; break;
        case '7': from_rank = RANK_7; break;
        case '8': from_rank = RANK_8; break;
        default:  from_rank = 0;      break;
    }
    switch (str[2])
    {
        case 'a': case 'A': to_file = FILE_A; break;
        case 'b': case 'B': to_file = FILE_B; break;
        case 'c': case 'C': to_file = FILE_C; break;
        case 'd': case 'D': to_file = FILE_D; break;
        case 'e': case 'E': to_file = FILE_E; break;
        case 'f': case 'F': to_file = FILE_F; break;
        case 'g': case 'G': to_file = FILE_G; break;
        case 'h': case 'H': to_file = FILE_H; break;
        default:            to_file = 0;      break;
    }
    switch (str[3])
    {
        case '1': to_rank = RANK_1; break;
        case '2': to_rank = RANK_2; break;
        case '3': to_rank = RANK_3; break;
        case '4': to_rank = RANK_4; break;
        case '5': to_rank = RANK_5; break;
        case '6': to_rank = RANK_6; break;
        case '7': to_rank = RANK_7; break;
        case '8': to_rank = RANK_8; break;
        default:  to_rank = 0;      break;
    }
    const square_t from = from_file & from_rank;
    const square_t to   = to_file   & to_rank;
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
        if (argc < 3)
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
        const size_t moves_size = argc - 3;
        if (argc > 3)
        {
            moves = malloc(moves_size * sizeof(Move)); // TODO failure
            const char** move_strs = (const char**)&argv[3];
            for (size_t i=0; i<moves_size; ++i)
            {
                const char* move_str = move_strs[i];
                moves[i] = string_to_move(move_str, strlen(move_str));
            }
        }
        perft_test(depth, moves, moves_size, PERFT_TEST_VERBOSE_ALL);
        free(moves);
    }
    return 0;
}

