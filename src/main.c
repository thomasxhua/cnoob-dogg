#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "board_state.h"

uint64_t random_number(const uint64_t n) {
    uint64_t r;
    do {
        r = ((uint64_t)rand() << 33) ^ ((uint64_t)rand() << 2) ^ (rand() & 3);
    } while (r > UINT64_MAX - (UINT64_MAX % (n+1)));
    return r % (n+1);
}


int main()
{
    BoardState state = {0};
    Bitboard* board = &state.board;
    state.fields |=
        BOARD_STATE_FIELDS_CASTLING_WK
        | BOARD_STATE_FIELDS_CASTLING_WQ
        | BOARD_STATE_FIELDS_CASTLING_BK
        | BOARD_STATE_FIELDS_CASTLING_BQ
        | BOARD_STATE_FIELDS_ACTIVE_COLOR_W;

#if 0
    board->k = E1|H8;
    board->w = E1|A7|B7;
    board->p = A7|B7;
#else
    bitboard_set_starting_position(board);
#endif

    srand((unsigned)time(NULL));
    for (uint64_t i=0; i<80 * 2; ++i)
    {
        Move moves[BOARD_STATE_MOVES_SIZE];
        const bool is_white = state.fields & BOARD_STATE_FIELDS_ACTIVE_COLOR_W;
        const size_t length = board_state_get_legal_moves(&state, is_white, moves, BOARD_STATE_MOVES_SIZE);

#if 1
        for (size_t j=0; j<length; ++j)
        {
            const square_t* piece_ptr = bitboard_get_piece_ptr(board, moves[j].from);
            char piece_char =
                (piece_ptr == &board->k) ? 'K'
                : (piece_ptr == &board->q) ? 'Q'
                : (piece_ptr == &board->r) ? 'R'
                : (piece_ptr == &board->b) ? 'B'
                : (piece_ptr == &board->n) ? 'N'
                : (piece_ptr == &board->p) ? 'P'
                : '?';
            char str_from_moves[SQUARE_TO_STRING_SIZE];
            char str_to_moves[SQUARE_TO_STRING_SIZE];
            char str_queening[3] = {0};
            square_to_string(moves[j].from, str_from_moves, SQUARE_TO_STRING_SIZE);
            square_to_string(moves[j].to, str_to_moves, SQUARE_TO_STRING_SIZE);
            if (moves[j].fields == MOVE_FIELDS_QUEENING_CHOICE_Q)
            {
                str_queening[0] = '=';
                str_queening[1] = 'Q';
            }
            if (moves[j].fields == MOVE_FIELDS_QUEENING_CHOICE_R)
            {
                str_queening[0] = '=';
                str_queening[1] = 'R';
            }
            if (moves[j].fields == MOVE_FIELDS_QUEENING_CHOICE_B)
            {
                str_queening[0] = '=';
                str_queening[1] = 'B';
            }
            if (moves[j].fields == MOVE_FIELDS_QUEENING_CHOICE_N)
            {
                str_queening[0] = '=';
                str_queening[1] = 'N';
            }
            printf("%c%s-%s%s, ", piece_char, str_from_moves, str_to_moves, str_queening);
        }
#endif

        const uint64_t idx               = random_number(length-1);
        const square_t* piece_ptr        = bitboard_get_piece_ptr(board, moves[idx].from);
        const apply_move_status_t status = board_state_apply_move(&state, &moves[idx]);

        square_t annotation = board_state_get_attacked_kings(&state, is_white);

        char str_bitboard[BITBOARD_TO_STRING_SIZE];
        char str_fen[BOARD_STATE_TO_FEN_STRING_SIZE];
        bitboard_to_string_annotated(board, annotation, str_bitboard, BITBOARD_TO_STRING_SIZE);
        board_state_to_fen_string(&state, str_fen, BOARD_STATE_TO_FEN_STRING_SIZE);
        printf("\n%s\nFEN: \'%s\'",
            str_bitboard,
            str_fen);
        char str_from_moves[SQUARE_TO_STRING_SIZE];
        char str_to_moves[SQUARE_TO_STRING_SIZE];
        square_to_string(moves[idx].from, str_from_moves, SQUARE_TO_STRING_SIZE);
        square_to_string(moves[idx].to, str_to_moves, SQUARE_TO_STRING_SIZE);
        char piece_char =
            (piece_ptr == &board->k) ? 'K'
            : (piece_ptr == &board->q) ? 'Q'
            : (piece_ptr == &board->r) ? 'R'
            : (piece_ptr == &board->b) ? 'B'
            : (piece_ptr == &board->n) ? 'N'
            : (piece_ptr == &board->p) ? 'P'
            : '?';
        printf(" (%c%s-%s)\n\n", piece_char, str_from_moves, str_to_moves);

        switch (status)
        {
            case APPLY_MOVE_STATUS_ERROR_FROM_PIECE_EMPTY:
                printf("\n[ERROR] APPLY_MOVE_STATUS_ERROR_FROM_PIECE_EMPTY\n");
                return 0;
            case APPLY_MOVE_STATUS_ILLEGAL_ANY_KING_ATTACKED:
                printf("\n[ERROR] APPLY_MOVE_STATUS_ILLEGAL_ANY_KING_ATTACKED\n");
                return 0;
            default:
                break;
        }
    }
    return 0;
}

