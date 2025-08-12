#ifndef PERFT_H
#define PERFT_H

#include "board_state.h"
#include "board_state.h"

typedef enum
{
    PERFT_TEST_VERBOSE_NONE,
    PERFT_TEST_VERBOSE_TOTAL,
    PERFT_TEST_VERBOSE_ALL
} perft_test_verbose_t;

size_t perft_board_state_count(const BoardState* state, uint64_t depth);
size_t perft_board_state_test(const BoardState* state, uint64_t depth, bool is_printing_each_node);
size_t perft_test(uint64_t depth, Move* moves, size_t moves_size, perft_test_verbose_t verbose);

#endif // PERFT_H
