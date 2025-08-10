#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "board_state.h"

size_t perft_test(const BoardState* state, uint64_t depth)
{
    if (depth == 0)
        return 0;
    Move moves[BOARD_STATE_MOVES_SIZE];
    const bool is_white = state->fields & BOARD_STATE_FIELDS_ACTIVE_COLOR_W;
    const size_t count  = board_state_get_legal_moves(state, is_white, moves, BOARD_STATE_MOVES_SIZE);
    if (depth == 1)
        return count;
    size_t sum = 0;
    for (size_t i=0; i<count; ++i)
    {
        BoardState copy = {0};
        board_state_copy(state, &copy);
        const apply_move_status_t status = board_state_apply_move(&copy, &moves[i]);
        if (status != APPLY_MOVE_STATUS_OK)
        {
            printf("\n[ERROR] apply_move: %u", status);
            return 0;
        }
        sum += perft_test(&copy, depth-1);
    }
    return sum;
}

int main()
{
    BoardState state = {0};
    board_state_init(&state);
    // https://www.chessprogramming.org/Perft_Results
    for (uint64_t depth=0; depth<20; ++depth)
    {
        const size_t total_size = perft_test(&state, depth);
        printf("total_size(depth=%llu): %llu\n", depth, total_size);
    }
    return 0;
}

