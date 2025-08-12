#include "perft.h"

#include <assert.h>
#include <stdio.h>

size_t perft_board_state_count(const BoardState* state, uint64_t depth)
{
    assert(state != NULL);
    if (depth == 0)
        return 1;
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
        const size_t res = perft_board_state_count(&copy, depth-1);
        char from_str[SQUARE_TO_STRING_SIZE], to_str[SQUARE_TO_STRING_SIZE];
        square_to_string(moves[i].from, from_str, SQUARE_TO_STRING_SIZE);
        square_to_string(moves[i].to, to_str, SQUARE_TO_STRING_SIZE);
        sum += res;
    }
    return sum;
}

size_t perft_board_state_test(const BoardState* state, uint64_t depth, bool is_printing_each_node)
{
    assert(state != NULL);
    if (depth == 0)
        return 1;
    Move moves[BOARD_STATE_MOVES_SIZE];
    const bool is_white = state->fields & BOARD_STATE_FIELDS_ACTIVE_COLOR_W;
    const size_t count  = board_state_get_legal_moves(state, is_white, moves, BOARD_STATE_MOVES_SIZE);
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
        const size_t res = perft_board_state_count(&copy, depth-1);
        sum += res;
        char from_str[SQUARE_TO_STRING_SIZE], to_str[SQUARE_TO_STRING_SIZE];
        square_to_string(moves[i].from, from_str, SQUARE_TO_STRING_SIZE);
        square_to_string(moves[i].to, to_str, SQUARE_TO_STRING_SIZE);
        if (is_printing_each_node)
            printf("%s%s: %llu\n", from_str, to_str, res);
    }
    return sum;
}

size_t perft_test(uint64_t depth, Move* moves, size_t moves_size, perft_test_verbose_t verbose)
{
    BoardState state = {0};
    board_state_init(&state);
    if (moves)
    {
        for (size_t i=0; i<moves_size; ++i)
            board_state_apply_move(&state, &moves[i]);
    }
    const size_t total_size = perft_board_state_test(&state, depth, verbose == PERFT_TEST_VERBOSE_ALL);
    if (verbose != PERFT_TEST_VERBOSE_NONE)
        printf("total_size(depth=%llu): %llu\n", depth, total_size);
    return total_size;
}

size_t perft_test_fen(uint64_t depth, char* str, size_t str_size, perft_test_verbose_t verbose)
{
    BoardState state = {0};
    board_state_set_fen_string(&state, str, str_size);
    const size_t total_size = perft_board_state_test(&state, depth, verbose == PERFT_TEST_VERBOSE_ALL);
    if (verbose != PERFT_TEST_VERBOSE_NONE)
        printf("total_size(depth=%llu): %llu\n", depth, total_size);
    return total_size;
}

