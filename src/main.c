#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "bitboard.h"
#include "board_state.h"
#include "perft.h"
#include "uci.h"
#include "utils.h"
#include "dyn_array.h"

#define CNOOBDOGG_MANUAL \
    "Usage:\n" \
    "  perft <depth> moves...    Run perft depth after applying moves to starting position.\n" \
    "  fen moves...              Generate fen string after applying moves to starting position.\n" \
    "  perft-fen <fen> moves...  Run perft depth after applying moves to fen position.\n" \
    "  uci                       Start UCI mode.\n"

#define CNOOBDOGG_TYPE_HELP "Type 'help' for more information.\n"

#define MAIN_BUFFER_SIZE 256
#define MAIN_TOKENS_SIZE 64

void handle_fen(char** tokens, size_t tokens_size);
void handle_perft(char** tokens, size_t tokens_size);
void handle_perft_fen(char** tokens, size_t tokens_size);

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    printf("%s\n", UCI_CNOOBDOGG_NAME);
    for (;;)
    {
        printf("\n");
        dyn_array_char buffer;
        dyn_array_char_alloc(&buffer, MAIN_BUFFER_SIZE);
        int c = getc(stdin);
        for (; c && c != '\n' && c != EOF; c = getc(stdin))
            dyn_array_char_append(&buffer, (char)c);
        if (c == EOF && buffer.size == 0)
                break;
        dyn_array_char_append(&buffer, '\0');
        char* tokens[MAIN_TOKENS_SIZE];
        const size_t tokens_size = string_tokenize_alloc(buffer.data, tokens, MAIN_TOKENS_SIZE);
        const char* cmd = tokens[0];
        if (strcmp(cmd, "quit") == 0)
            break;
        else if (strcmp(cmd, "help") == 0)
            printf(CNOOBDOGG_MANUAL);
        else if (strcmp(cmd, "fen") == 0)
            handle_fen(tokens + 1, tokens_size - 1);
        else if (strcmp(cmd, "perft") == 0)
            handle_perft(tokens + 1, tokens_size - 1);
        else if (strcmp(cmd, "perft-fen") == 0)
            handle_perft_fen(tokens + 1, tokens_size - 1);
        else if (strcmp(cmd, "uci") == 0)
            uci_run_dialog();
        else
            printf("Unknown command '%s'. " CNOOBDOGG_TYPE_HELP, cmd);
        for (size_t i=0; i<tokens_size; ++i)
            free(tokens[i]);
        printf("\n");
    }
    return 0;
}

void handle_fen(char** tokens, size_t tokens_size)
{
    assert(tokens != NULL);
    Move* moves = NULL;
    const size_t moves_size = tokens_size;
    if (moves_size > 0)
    {
        moves = malloc(moves_size * sizeof(Move));
        for (size_t i=0; i<moves_size; ++i)
            moves[i] = string_to_move(tokens[i], strlen(tokens[i]));
    }
    BoardState state = {0};
    board_state_init(&state);
    for (size_t i=0; i<moves_size; ++i)
        board_state_apply_move(&state, &moves[i]);
    free(moves);
    char fen_str[BOARD_STATE_TO_FEN_STRING_SIZE];
    board_state_to_fen_string(&state, fen_str, BOARD_STATE_TO_FEN_STRING_SIZE);
    printf("%s\n", fen_str);
}

void handle_perft(char** tokens, size_t tokens_size)
{
    assert(tokens != NULL);
    if (tokens_size < 1)
    {
        printf("Please provide a depth. " CNOOBDOGG_TYPE_HELP);
        return;
    }
    const uint64_t depth = atoi(tokens[0]);
    Move* moves = NULL;
    const size_t moves_size = tokens_size - 1;
    if (moves_size > 0)
    {
        moves = malloc(moves_size * sizeof(Move));
        for (size_t i=0; i<moves_size; ++i)
            moves[i] = string_to_move(tokens[i+1], strlen(tokens[i+1]));
    }
    perft_test(depth, moves, moves_size, PERFT_TEST_VERBOSE_ALL);
    free(moves);
}

void handle_perft_fen(char** tokens, size_t tokens_size)
{
    assert(tokens != NULL);
    if (tokens_size < 2)
    {
        printf("Please provide a depth and fen string. " CNOOBDOGG_TYPE_HELP);
        return;
    }
    const uint64_t depth = atoi(tokens[1]);
    Move* moves = NULL;
    const size_t moves_size = tokens_size - 2;
    const char* fen_str = tokens[0];
    if (moves_size > 0)
    {
        BoardState state = {0};
        board_state_set_fen_string(&state, fen_str, BOARD_STATE_SET_FEN_STRING_SIZE);
        moves = malloc(moves_size * sizeof(Move));
        for (size_t i=0; i<moves_size; ++i)
            moves[i] = string_to_move(tokens[i+2], strlen(tokens[i+2]));
        for (size_t i=0; i<moves_size; ++i)
            board_state_apply_move(&state, &moves[i]);
        free(moves);
        char fen_str_moved[BOARD_STATE_SET_FEN_STRING_SIZE];
        board_state_to_fen_string(&state, fen_str_moved, BOARD_STATE_TO_FEN_STRING_SIZE);
        perft_test_fen(depth, fen_str_moved, BOARD_STATE_TO_FEN_STRING_SIZE, PERFT_TEST_VERBOSE_ALL);
    }
    else
    {
        perft_test_fen(depth, fen_str, BOARD_STATE_TO_FEN_STRING_SIZE, PERFT_TEST_VERBOSE_ALL);
    }
    free(moves);
}

