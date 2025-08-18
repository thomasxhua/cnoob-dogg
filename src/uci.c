#include "uci.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "utils.h"
#include "dyn_array.h"

void uci_state_init_alloc(UCIState* uci)
{
    assert(uci != NULL);
    board_state_init(&uci->state);
    dyn_array_pthread_alloc(&uci->threads, 8);
    uci->search_mode = UCI_SEARCH_MODE_STOP;
    uci->debug_flag  = false;
}

void uci_run_dialog(void)
{
    uci_cmd_uci();
    UCIState uci = {0};
    uci_state_init_alloc(&uci);
    for (;;)
    {
        printf("\n");
        dyn_array_char buffer;
        dyn_array_char_alloc(&buffer, UCI_BUFFER_SIZE);
        int c = getc(stdin);
        for (; c && c != '\n' && c != EOF; c = getc(stdin))
            dyn_array_char_append(&buffer, (char)c);
        if (c == EOF && buffer.size == 0)
                break;
        dyn_array_char_append(&buffer, '\0');
        dyn_array_char_ptr tokens;
        dyn_array_char_ptr_alloc(&tokens, UCI_TOKENS_SIZE);
        string_tokenize_alloc(buffer.data, &tokens);
        char** tokens_in            = tokens.data+1;
        const size_t tokens_in_size = tokens.size-1;
        const char* cmd = tokens.data[0];
        if (strcmp(cmd, "uci") == 0)
            uci_cmd_uci();
        else if (strcmp(cmd, "debug") == 0)
            uci_cmd_debug(&uci, tokens_in, tokens_in_size);
        else if (strcmp(cmd, "isready") == 0)
            uci_cmd_isready();
        else if (strcmp(cmd, "setoption") == 0)
            assert(false && "NOT IMPLEMENTED");
        else if (strcmp(cmd, "position") == 0)
            uci_cmd_position(&uci, tokens_in, tokens_in_size);
        else if (strcmp(cmd, "go") == 0)
            uci_cmd_go(&uci, tokens_in, tokens_in_size);
        else if (strcmp(cmd, "stop") == 0)
            uci_cmd_stop(&uci);
        else if (strcmp(cmd, "ponderhit") == 0)
            assert(false && "NOT IMPLEMENTED");
        else if (strcmp(cmd, "quit") == 0)
            break;
        for (size_t i=0; i<tokens.size; ++i)
            free(tokens.data[i]);
        dyn_array_char_free(&buffer);
        dyn_array_char_ptr_free(&tokens);
    }
    dyn_array_pthread_free(&uci.threads);
}

void uci_cmd_uci(void)
{
    uci_send_id();
    uci_send_uciok();
}

void uci_cmd_debug(UCIState* uci, char** tokens, size_t tokens_size)
{
    assert(uci != NULL);
    assert(tokens != NULL);
    assert(tokens_size >= 1);
    if (strcmp(tokens[0], "on") == 0)
        uci->debug_flag = true;
    else if (strcmp(tokens[0], "off") == 0)
        uci->debug_flag = false;
}

void uci_cmd_isready(void)
{
    uci_send_readyok();
}

void uci_cmd_position(UCIState* uci, char** tokens, size_t tokens_size)
{
    assert(uci != NULL);
    assert(tokens != NULL);
    size_t moves_token_idx = 1;
    BoardState* state = &uci->state;
    if (strcmp(tokens[0], "fen") == 0)
    {
        moves_token_idx = 2;
        board_state_set_fen_string(state, tokens[1], BOARD_STATE_SET_FEN_STRING_SIZE);
    }
    else if (strcmp(tokens[0], "startpos") == 0)
    {
        board_state_init(state);
    }
    if (tokens_size > moves_token_idx && strcmp(tokens[moves_token_idx], "moves") == 0)
    {
        const size_t moves_size = tokens_size - moves_token_idx - 1;
        if (moves_size > 0)
        {
            for (size_t i=1; i<=moves_size; ++i)
            {
                const char* move_token = tokens[i+moves_token_idx];
                const Move move        = string_to_move(move_token, strlen(move_token));
                board_state_apply_move(&uci->state, &move);
            }
        }
    }
}

void uci_cmd_go(UCIState* uci, char** tokens, size_t tokens_size)
{
    assert(uci != NULL);
    for (size_t i=0; i<tokens_size; ++i)
    {
        char* token = tokens[i];
        if (strcmp(token, "searchmoves") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "ponder") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "wtime") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "btime") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "winc") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "binc") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "movestogo") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "depth") == 0)
        {
            if (++i >= tokens_size)
                break;
            uci->depth = atoi(tokens[i]); // TODO handle failure
            uci->search_mode = UCI_SEARCH_MODE_DEPTH;
        }
        else if (strcmp(token, "nodes") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "mate") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "movetime") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
        }
        else if (strcmp(token, "infinite") == 0)
        {
            assert(false && "NOT IMPLEMENTED");
            uci->search_mode = UCI_SEARCH_MODE_INFINITE;
        }
    }
    pthread_create(&uci->threads.data[0], NULL, uci_search_loop, uci);
    pthread_join(uci->threads.data[0], NULL);
    uci_send_bestmove(uci);
}

void uci_cmd_stop(UCIState* uci)
{
    assert(uci != NULL);
    assert(uci->search_mode != UCI_SEARCH_MODE_STOP);
    uci->search_mode = UCI_SEARCH_MODE_STOP;
    char bestmove_str[MOVE_TO_STRING_SIZE];
    move_to_string(&uci->bestmove, bestmove_str, MOVE_TO_STRING_SIZE);
    printf("bestmove %s\n", bestmove_str);
}

void uci_send_id(void)
{
    printf("id name " UCI_CNOOBDOGG_NAME "\n");
    printf("id author " UCI_CNOOBDOGG_AUTHOR "\n");
}

void uci_send_uciok(void)
{
    printf("uciok\n");
}

void uci_send_readyok(void)
{
    printf("readyok\n");
}

void uci_send_bestmove(UCIState* uci)
{
    assert(uci != NULL);
    char bestmove_str[MOVE_TO_STRING_SIZE];
    move_to_string(&uci->bestmove, bestmove_str, MOVE_TO_STRING_SIZE);
    printf("bestmove %s\n", bestmove_str);
}

void* uci_search_loop(void* arg)
{
    assert(arg != NULL);
    UCIState* uci           = arg;
    const BoardState* state = &uci->state;
    Move moves[BOARD_STATE_MOVES_SIZE];
    const size_t moves_size = board_state_get_legal_moves(state, moves, BOARD_STATE_MOVES_SIZE);
    if (moves_size == 0)
        return NULL;
    switch (uci->search_mode)
    {
        case UCI_SEARCH_MODE_DEPTH:
        {
            evaluation_t max_evaluation = -INFINITY;
            for (size_t i=0; i<moves_size; ++i)
            {
                const Move* move = &moves[i];
                BoardState copy  = {0};
                board_state_copy(state, &copy);
                board_state_apply_move(&copy, move);
                const evaluation_t evaluation = -board_state_evaluate_minimax(&copy, uci->depth);
                if (evaluation > max_evaluation)
                {
                    max_evaluation = evaluation;
                    uci->bestmove  = *move;
                }
            }
            break;
        }
        default: break;
    }
    char bestmove_str[MOVE_TO_STRING_SIZE];
    move_to_string(&uci->bestmove, bestmove_str, MOVE_TO_STRING_SIZE);
    return NULL;
}

