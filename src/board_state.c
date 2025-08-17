#include "board_state.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "utils.h"

void board_state_init(BoardState* state)
{
    state->fields |=
        BOARD_STATE_FIELDS_CASTLING_WK
        | BOARD_STATE_FIELDS_CASTLING_WQ
        | BOARD_STATE_FIELDS_CASTLING_BK
        | BOARD_STATE_FIELDS_CASTLING_BQ
        | BOARD_STATE_FIELDS_ACTIVE_COLOR_W;
    state->fullmove_count = 1;
    bitboard_set_starting_position(&state->board);
}

void board_state_clear(BoardState* state)
{
    state->board             = (Bitboard){0};
    state->fullmove_count    = 0;
    state->halfmove_clock    = 0;
    state->en_passant_square = 0;
    state->fields            = 0;
}

void board_state_to_fen_string(const BoardState* state, char* str, size_t str_size)
{
    assert(state != NULL);
    assert(str != NULL);
    assert(str_size >= BOARD_STATE_TO_FEN_STRING_SIZE);
    const Bitboard* board = &state->board;
    // piece placement data
    uint64_t empty_counter = 0;
    uint64_t rank_num = RANK_SIZE;
    square_t square   = 1ULL << (rank_num * RANK_SIZE);
    size_t idx        = 0;
    for (uint64_t i=0; i<BOARD_SIZE; ++i)
    {
        if (i % RANK_SIZE == 0)
        {
            if (i > 0)
            {
                if (empty_counter)
                {
                    str[idx++] = '0' + empty_counter;
                    empty_counter = 0;
                }
                str[idx++] = '/';
            }
            square = 1ULL << (--rank_num * RANK_SIZE);
        }
        const piece_t piece = bitboard_get_piece(board, square);
        if (piece != PIECE_NONE)
        {
            if (empty_counter)
            {
                str[idx++] = '0' + empty_counter;
                empty_counter = 0;
            }
            str[idx++] = piece_to_char(piece);
            empty_counter = 0;
        }
        else
        {
            ++empty_counter;
        }
        square <<= 1;
    }
    if (empty_counter)
    {
        str[idx++] = '0' + empty_counter;
        empty_counter = 0;
    }
    // active color
    str[idx++] = ' ';
    str[idx++] = (state->fields & BOARD_STATE_FIELDS_ACTIVE_COLOR_W) ? 'w' : 'b';
    // castling
    str[idx++] = ' ';
    if (state->fields & BOARD_STATE_FIELDS_CASTLING)
    {
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_WK) str[idx++] = 'K';
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_WQ) str[idx++] = 'Q';
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_BK) str[idx++] = 'k';
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_BQ) str[idx++] = 'q';
    }
    else
    {
        str[idx++] = '-';
    }
    // en passant square
    str[idx++] = ' ';
    if (state->en_passant_square)
    {
        char square_str[SQUARE_TO_STRING_SIZE];
        square_to_string(state->en_passant_square, square_str, SQUARE_TO_STRING_SIZE);
        str[idx++] = square_str[0];
        str[idx++] = square_str[1];
    }
    else
    {
        str[idx++] = '-';
    }
    // halfmove clock
    str[idx++] = ' ';
    char halfmove_str[UINT64_TO_STRING_SIZE];
    uint64_to_string(state->halfmove_clock, halfmove_str, UINT64_TO_STRING_SIZE);
    for (char* ptr = halfmove_str; *ptr != '\0'; ++ptr)
        str[idx++] = *ptr;
    // fullmove count
    str[idx++] = ' ';
    char fullmove_str[UINT64_TO_STRING_SIZE];
    uint64_to_string(state->fullmove_count, fullmove_str, UINT64_TO_STRING_SIZE);
    for (char* ptr = fullmove_str; *ptr != '\0'; ++ptr)
        str[idx++] = *ptr;
    str[idx++] = '\0';
}

void board_state_set_fen_string(BoardState* state, const char* str, size_t str_size)
{
    assert(state != NULL);
    board_state_clear(state);
    Bitboard* board = &state->board;
    uint64_t rank_num = RANK_SIZE - 1;
    square_t square = 1ULL << (rank_num * RANK_SIZE);
    bool is_board_data_done=false, is_active_color_done=false, is_castling_done=false;
    for (size_t i=0; i<str_size; ++i)
    {
        if (!is_board_data_done)
        {
            switch (str[i])
            {
                // white piece
                case 'K': bitboard_place_piece(board, square, &board->k, true); square <<= 1; break;
                case 'Q': bitboard_place_piece(board, square, &board->q, true); square <<= 1; break;
                case 'R': bitboard_place_piece(board, square, &board->r, true); square <<= 1; break;
                case 'B': bitboard_place_piece(board, square, &board->b, true); square <<= 1; break;
                case 'N': bitboard_place_piece(board, square, &board->n, true); square <<= 1; break;
                case 'P': bitboard_place_piece(board, square, &board->p, true); square <<= 1; break;
                // black piece
                case 'k': bitboard_place_piece(board, square, &board->k, false); square <<= 1; break;
                case 'q': bitboard_place_piece(board, square, &board->q, false); square <<= 1; break;
                case 'r': bitboard_place_piece(board, square, &board->r, false); square <<= 1; break;
                case 'b': bitboard_place_piece(board, square, &board->b, false); square <<= 1; break;
                case 'n': bitboard_place_piece(board, square, &board->n, false); square <<= 1; break;
                case 'p': bitboard_place_piece(board, square, &board->p, false); square <<= 1; break;
                // empty
                case '/': square = 1ULL << (--rank_num * RANK_SIZE); break;
                case ' ': is_board_data_done = true; break;
                default:
                {
                    if (str[i] >= '1' && str[i] <= '8')
                        square <<= str[i] - '0';
                    else
                        return;
                    break;
                }
            }
        }
        else if (!is_active_color_done)
        {
            switch (str[i])
            {
                case 'w': state->fields |= BOARD_STATE_FIELDS_ACTIVE_COLOR_W; break;
                case 'b': state->fields &= ~BOARD_STATE_FIELDS_ACTIVE_COLOR_W; break;
                case ' ': is_active_color_done = true; break;
                default:  break;
            }
        }
        else if (!is_castling_done)
        {
            switch (str[i])
            {
                case 'K': state->fields |= BOARD_STATE_FIELDS_CASTLING_WK; break;
                case 'Q': state->fields |= BOARD_STATE_FIELDS_CASTLING_WQ; break;
                case 'k': state->fields |= BOARD_STATE_FIELDS_CASTLING_BK; break;
                case 'q': state->fields |= BOARD_STATE_FIELDS_CASTLING_BQ; break;
                case '-': state->fields &= ~BOARD_STATE_FIELDS_CASTLING; is_castling_done = true; break;
                case ' ': is_castling_done = true; break;
                default: break;
            }
        }
        else
        {
            state->en_passant_square = string_to_square(str + i, STRING_TO_SQUARE_SIZE);
            int halfmove=0, fullmove=0;
            sscanf(str + i + STRING_TO_SQUARE_SIZE, "%d %d", &halfmove, &fullmove);
            state->halfmove_clock = halfmove;
            state->fullmove_count = fullmove;
            return;
        }
    }
}

void board_state_copy(const BoardState* state, BoardState* other)
{
    assert(state != NULL);
    assert(other != NULL);
    memcpy(other, state, sizeof(BoardState));
}

void board_state_print(const BoardState* state, const square_t annotation)
{
    assert(state != NULL);
    char board_str[BITBOARD_TO_STRING_SIZE];
    bitboard_to_string_annotated(&state->board, annotation, board_str, BITBOARD_TO_STRING_SIZE);
    char fen_str[BOARD_STATE_TO_FEN_STRING_SIZE];
    board_state_to_fen_string(state, fen_str, BOARD_STATE_TO_FEN_STRING_SIZE);
    printf("%s\n%s; eval: %.2f\n", board_str, fen_str, board_state_evaluate(state));
}

bool board_state_is_white(const BoardState* state)
{
    return state->fields & BOARD_STATE_FIELDS_ACTIVE_COLOR_W;   
}

square_t board_state_get_pseudo_legal_squares_pawns_attacks_no_en_passant(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board          = &state->board;
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t opponent_pieces = all_pieces & (is_white ? ~board->w : board->w);
    const square_t own_pawns       = selection & board->p & (is_white ? board->w : ~board->w);
    const square_t pawns_a         = own_pawns & FILE_A;
    const square_t pawns_h         = own_pawns & FILE_H;
    const square_t pawns_inner     = own_pawns & ~(FILE_A|FILE_H);
    const square_t attacks         = is_white
        ? opponent_pieces & (
            pawns_a       << (RANK_SIZE + 1)
            | pawns_inner << (RANK_SIZE + 1)
            | pawns_inner << (RANK_SIZE - 1)
            | pawns_h     << (RANK_SIZE - 1))
        : opponent_pieces & (
            pawns_a       >> (RANK_SIZE - 1)
            | pawns_inner >> (RANK_SIZE - 1)
            | pawns_inner >> (RANK_SIZE + 1)
            | pawns_h     >> (RANK_SIZE + 1));
    return attacks;
}

square_t board_state_get_pseudo_legal_squares_pawns_moves(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board          = &state->board;
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t own_pawns       = selection & board->p & (is_white ? board->w : ~board->w);
    const square_t moves_once      = is_white
        ? ~all_pieces & (own_pawns << RANK_SIZE)
        : ~all_pieces & (own_pawns >> RANK_SIZE);
    const square_t moves_twice     = is_white
        ? ~all_pieces & ((~all_pieces & ((own_pawns & RANK_2) << RANK_SIZE)) << RANK_SIZE)
        : ~all_pieces & ((~all_pieces & ((own_pawns & RANK_7) >> RANK_SIZE)) >> RANK_SIZE);
    return moves_once | moves_twice; /* TODO: en passant and selection dont line up */
}

square_t board_state_get_pseudo_legal_squares_knights(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board      = &state->board;
    const square_t own_knights = selection & board->n & (is_white ? board->w : ~board->w);
    const square_t own_pieces  = bitboard_get_all_pieces(board) & (is_white ? board->w : ~board->w);
    //   .NW .NE .
    //  WN . . .EN
    //   . .N. . .
    //  WS . . .ES
    //   .SW .SE .
    return ~own_pieces & (
        (own_knights   & ~(FILE_A        | RANK_8|RANK_7)) << ((2 * RANK_SIZE) - 1)   // NW
        | (own_knights & ~(FILE_H        | RANK_8|RANK_7)) << ((2 * RANK_SIZE) + 1)   // NE
        | (own_knights & ~(FILE_A|FILE_B | RANK_8       )) << ((1 * RANK_SIZE) - 2)   // WN
        | (own_knights & ~(FILE_G|FILE_H | RANK_8       )) << ((1 * RANK_SIZE) + 2)   // EN
        | (own_knights & ~(FILE_A|FILE_B | RANK_1       )) >> ((1 * RANK_SIZE) + 2)   // WS
        | (own_knights & ~(FILE_G|FILE_H | RANK_1       )) >> ((1 * RANK_SIZE) - 2)   // ES
        | (own_knights & ~(FILE_A        | RANK_1|RANK_2)) >> ((2 * RANK_SIZE) + 1)   // SW
        | (own_knights & ~(FILE_H        | RANK_1|RANK_2)) >> ((2 * RANK_SIZE) - 1)); // SE
}

square_t get_pseudo_legal_squares_bishops(const BoardState* state, bool is_white, square_t piece)
{
    assert(state != NULL);
    const Bitboard* board          = &state->board;
    const square_t own_bishops     = piece & (is_white ? board->w : ~board->w);
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t own_pieces      = all_pieces & (is_white ? board->w : ~board->w);
    const square_t opponent_pieces = all_pieces & (is_white ? ~board->w : board->w);
    square_t moves = 0;
    for (square_t square = 1ULL; square; square <<= 1)
    {
        if (!(own_bishops & square))
            continue;
        square_t temp;
        // NW
        temp = square;
        for (;;)
        {
            if (temp & (RANK_8|FILE_A)) break;
            temp <<= RANK_SIZE - 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // NE
        temp = square;
        for (;;)
        {
            if (temp & (RANK_8|FILE_H)) break;
            temp <<= RANK_SIZE + 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // SW
        temp = square;
        for (;;)
        {
            if (temp & (RANK_1|FILE_A)) break;
            temp >>= RANK_SIZE + 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // SE
        temp = square;
        for (;;)
        {
            if (temp & (RANK_1|FILE_H)) break;
            temp >>= RANK_SIZE - 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
    }
    return moves;
}

square_t board_state_get_pseudo_legal_squares_bishops(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    return get_pseudo_legal_squares_bishops(state, is_white, selection & state->board.b);
}

square_t get_pseudo_legal_squares_rooks(const BoardState* state, bool is_white, square_t piece)
{
    assert(state != NULL);
    const Bitboard* board          = &state->board;
    const square_t own_rooks       = piece & (is_white ? board->w : ~board->w);
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t own_pieces      = all_pieces & (is_white ? board->w : ~board->w);
    const square_t opponent_pieces = all_pieces & (is_white ? ~board->w : board->w);
    square_t moves = 0;
    for (square_t square = 1ULL; square; square <<= 1)
    {
        if (!(own_rooks & square))
            continue;
        square_t temp;
        // N
        temp = square;
        for (;;)
        {
            if (temp & RANK_8) break;
            temp <<= RANK_SIZE;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // E
        temp = square;
        for (;;)
        {
            if (temp & FILE_H) break;
            temp <<= 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // S
        temp = square;
        for (;;)
        {
            if (temp & RANK_1) break;
            temp >>= RANK_SIZE;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
        // W
        temp = square;
        for (;;)
        {
            if (temp & FILE_A) break;
            temp >>= 1;
            if (temp & own_pieces) break;
            moves |= temp;
            if (temp & opponent_pieces) break;
        }
    }
    return moves;
}

square_t board_state_get_pseudo_legal_squares_rooks(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    return get_pseudo_legal_squares_rooks(state, is_white, selection & state->board.r);
}

square_t board_state_get_pseudo_legal_squares_queens(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    return
        get_pseudo_legal_squares_bishops(state, is_white, selection & state->board.q)
        | get_pseudo_legal_squares_rooks(state, is_white, selection & state->board.q);
}

square_t board_state_get_pseudo_legal_squares_kings(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board     = &state->board;
    const square_t own_kings  = selection & board->k & (is_white ? board->w : ~board->w);
    const square_t all_pieces = bitboard_get_all_pieces(board);
    const square_t own_pieces = all_pieces & (is_white ? board->w : ~board->w);
    square_t moves = 0;
    // 1.2.3.
    // 4.K.5.
    // 6.7.8.
    for (square_t square = 1ULL; square; square <<= 1)
    {
        if (!(own_kings & square))
            continue;
        moves |=
            ((square   & (RANK_8|FILE_A)) ? 0 : (square << (RANK_SIZE - 1)))  // 1
            | ((square & (RANK_8       )) ? 0 : (square << (RANK_SIZE)))      // 2
            | ((square & (RANK_8|FILE_H)) ? 0 : (square << (RANK_SIZE + 1)))  // 3
            | ((square & (       FILE_A)) ? 0 : (square >> 1))                // 4
            | ((square & (       FILE_H)) ? 0 : (square << 1))                // 5
            | ((square & (RANK_1|FILE_A)) ? 0 : (square >> (RANK_SIZE + 1)))  // 6
            | ((square & (RANK_1       )) ? 0 : (square >> (RANK_SIZE)))      // 7
            | ((square & (RANK_1|FILE_H)) ? 0 : (square >> (RANK_SIZE - 1))); // 8
    }
    moves &= ~own_pieces;
    // castling rules are checked here
    BoardState copy = {0};
    if (is_white)
    {
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_WQ)
        {
            copy.board = state->board;
            const square_t pseudo_kings = C1|D1|E1;
            copy.board.k |= pseudo_kings;
            copy.board.w |= pseudo_kings;
            if (!(all_pieces & (B1|C1|D1)) && !board_state_get_attacked_kings(&copy, is_white))
                moves |= C1;
        }
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_WK)
        {
            copy.board = state->board;
            const square_t pseudo_kings = E1|F1|G1;
            copy.board.k |= pseudo_kings;
            copy.board.w |= pseudo_kings;
            if (!(all_pieces & (F1|G1)) && !board_state_get_attacked_kings(&copy, is_white))
                moves |= G1;
        }
    }
    else
    {
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_BQ)
        {
            copy.board = state->board;
            const square_t pseudo_kings = C8|D8|E8;
            copy.board.k |= pseudo_kings;
            if (!(all_pieces & (B8|C8|D8)) && !board_state_get_attacked_kings(&copy, is_white))
                moves |= C8;
        }
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_BK)
        {
            copy.board = state->board;
            const square_t pseudo_kings = E8|F8|G8;
            copy.board.k |= pseudo_kings;
            if (!(all_pieces & (F8|G8)) && !board_state_get_attacked_kings(&copy, is_white))
                moves |= G8;
        }
    }
    return moves;
}

size_t board_state_get_pseudo_legal_moves_pawns(const BoardState* state, Move* moves, size_t moves_size)
{
    assert(state != NULL);
    assert(moves != NULL);
    assert(moves_size >= BOARD_STATE_MOVES_PIECES_SIZE);
    const Bitboard* board     = &state->board;
    const bool is_white       = board_state_is_white(state);
    const square_t own_pawns = board->p & (is_white ? board->w : ~board->w);
    size_t idx = 0;
    for (square_t from = 1ULL; from; from <<= 1)
    {
        if (!(from & own_pawns))
            continue;
        square_t square_moves = board_state_get_pseudo_legal_squares_pawns_moves(state, is_white, from);
        BoardState copy = {0};
        board_state_copy(state, &copy);
        copy.board.p |= state->en_passant_square;
        if (is_white)
            copy.board.w &= ~state->en_passant_square;
        else
            copy.board.w |= state->en_passant_square;
        square_moves |= board_state_get_pseudo_legal_squares_pawns_attacks_no_en_passant(&copy, is_white, from);
        for (square_t to = 1ULL; to; to <<= 1)
        {
            if (!(to & square_moves))
                continue;
            if (is_white && (to & RANK_8))
            {
                moves[idx++] = (Move) { .from=from, .to=to, .fields=MOVE_FIELDS_QUEENING_CHOICE_Q };
                moves[idx++] = (Move) { .from=from, .to=to, .fields=MOVE_FIELDS_QUEENING_CHOICE_R };
                moves[idx++] = (Move) { .from=from, .to=to, .fields=MOVE_FIELDS_QUEENING_CHOICE_B };
                moves[idx++] = (Move) { .from=from, .to=to, .fields=MOVE_FIELDS_QUEENING_CHOICE_N };
            }
            else if (!is_white && (to & RANK_1))
            {
                moves[idx++] = (Move) { .from=from, .to=to, .fields=MOVE_FIELDS_QUEENING_CHOICE_Q };
                moves[idx++] = (Move) { .from=from, .to=to, .fields=MOVE_FIELDS_QUEENING_CHOICE_R };
                moves[idx++] = (Move) { .from=from, .to=to, .fields=MOVE_FIELDS_QUEENING_CHOICE_B };
                moves[idx++] = (Move) { .from=from, .to=to, .fields=MOVE_FIELDS_QUEENING_CHOICE_N };
            }
            else
            {
                moves[idx++] = (Move)
                {
                    .from = from,
                    .to   = to
                };
            }
        }
    }
    return idx;
}

#define BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(name, lowercase) \
    size_t board_state_get_pseudo_legal_moves_##name(const BoardState* state, Move* moves, size_t moves_size) \
    { \
        assert(state != NULL); \
        assert(moves != NULL); \
        assert(moves_size >= BOARD_STATE_MOVES_PIECES_SIZE); \
        const Bitboard* board     = &state->board; \
        const bool is_white       = board_state_is_white(state); \
        const square_t own_##name = board->lowercase & (is_white ? board->w : ~board->w); \
        size_t idx = 0; \
        for (square_t from = 1ULL; from; from <<= 1) \
        { \
            if (!(from & own_##name)) \
                continue; \
            const square_t square_moves = board_state_get_pseudo_legal_squares_##name(state, is_white, from); \
            for (square_t to = 1ULL; to; to <<= 1) \
            { \
                if (!(to & square_moves)) \
                    continue; \
                moves[idx++] = (Move) \
                { \
                    .from = from, \
                    .to   = to \
                }; \
            } \
        } \
        return idx; \
    }
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(knights, n)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(bishops, b)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(rooks, r)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(queens, q)
BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE(kings, k)
#undef BOARD_STATE_GET_PSEUDO_LEGAL_MOVES_PIECE

size_t board_state_get_pseudo_legal_moves(const BoardState* state, Move* moves, size_t moves_size)
{
    assert(state != NULL);
    assert(moves != NULL);
    assert(moves_size >= BOARD_STATE_MOVES_SIZE);
    size_t idx = 0;
    idx += board_state_get_pseudo_legal_moves_pawns(state, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_knights(state, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_bishops(state, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_rooks(state, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_queens(state, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_kings(state, moves + idx, moves_size);
    return idx;
}

square_t board_state_get_attacked_kings(const BoardState* state, bool is_white)
{
    assert(state != NULL);
    const Bitboard* board        = &state->board;
    const square_t moves_pawns   = board_state_get_pseudo_legal_squares_pawns_attacks_no_en_passant(state, !is_white, BOARD_FULL);
    const square_t moves_knights = board_state_get_pseudo_legal_squares_knights(state, !is_white, BOARD_FULL);
    const square_t moves_bishops = board_state_get_pseudo_legal_squares_bishops(state, !is_white, BOARD_FULL);
    const square_t moves_rooks   = board_state_get_pseudo_legal_squares_rooks(state, !is_white, BOARD_FULL);
    const square_t moves_queens  = board_state_get_pseudo_legal_squares_queens(state, !is_white, BOARD_FULL);
    const square_t moves_kings   = board_state_get_pseudo_legal_squares_kings(state, !is_white, BOARD_FULL);
    const square_t own_kings     = board->k & (is_white ? board->w : ~board->w);
    return own_kings & (
        moves_pawns
        | moves_knights
        | moves_bishops
        | moves_rooks
        | moves_queens
        | moves_kings);
}

apply_move_status_t board_state_apply_move(BoardState* state, const Move* move)
{
    assert(state != NULL);
    assert(move != NULL);
    Bitboard* board            = &state->board;
    const square_t* from_piece = bitboard_get_piece_ptr_const(board, move->from);
    if (!from_piece)
        return APPLY_MOVE_STATUS_ERROR_FROM_PIECE_EMPTY;
    const bool is_white = board_state_is_white(state);
    bool is_capture = bitboard_get_piece_ptr(board, move->to);
    // -- handle move --
    if (from_piece == &board->p)
    {
        square_t* queening_piece =
            (move->fields & MOVE_FIELDS_QUEENING_CHOICE_Q)   ? &board->q
            : (move->fields & MOVE_FIELDS_QUEENING_CHOICE_R) ? &board->r
            : (move->fields & MOVE_FIELDS_QUEENING_CHOICE_B) ? &board->b
            : (move->fields & MOVE_FIELDS_QUEENING_CHOICE_N) ? &board->n
            : &board->q;
        // handle en passant active
        if (state->en_passant_square
            && (move->to == state->en_passant_square))
        {
            const square_t en_passant_target = is_white
                ? (state->en_passant_square >> RANK_SIZE)
                : (state->en_passant_square << RANK_SIZE);
            bitboard_clear_square(board, en_passant_target);
            bitboard_move_square(board, move->from, move->to);
            state->en_passant_square = 0;
            is_capture = true;
        }
        // handle pawn promotion
        else if (is_white && (move->to & RANK_8))
        {
            bitboard_clear_square(board, move->from);
            bitboard_clear_square(board, move->to);
            bitboard_place_piece(board, move->to, queening_piece, true);
        }
        else if (!is_white && (move->to & RANK_1))
        {
            bitboard_clear_square(board, move->from);
            bitboard_clear_square(board, move->to);
            bitboard_place_piece(board, move->to, queening_piece, false);
        }
        else
        {
            bitboard_move_square(board, move->from, move->to);
        }
    }
    // handle castling
    else if (from_piece == &board->k && square_log2_diff(move->from, move->to) == 2)
    {
        #define HANDLE_CASTLING(cond, direction, clear_range_min, clear_range_max, king_square, rook_square) \
            if (cond && move->from direction move->to) \
            { \
                for (square_t square = clear_range_min; square != clear_range_max << 1; square <<= 1) \
                    bitboard_clear_square(board, square); \
                bitboard_place_piece(board, king_square, &board->k, is_white); \
                bitboard_place_piece(board, rook_square, &board->r, is_white); \
            }
        HANDLE_CASTLING(is_white, >, A1, E1, C1, D1)      // WQ
        else HANDLE_CASTLING(!is_white, >, A8, E8, C8, D8) // BQ
        else HANDLE_CASTLING(is_white, <, E1, H1, G1, F1) // WK
        else HANDLE_CASTLING(!is_white, <, E8, H8, G8, F8) // BK
        else bitboard_move_square(board, move->from, move->to);
        #undef HANDLE_CASTLING
    }
    // handle normal move
    else
    {
        bitboard_move_square(board, move->from, move->to);
    }
    // -- set board state data --
    // fullmove
    if (is_white)
        state->fullmove_count += 1;
    state->fields ^= BOARD_STATE_FIELDS_ACTIVE_COLOR_W;
    // halfmove
    if (from_piece == &board->p || is_capture)
        state->halfmove_clock = 0;
    else
        ++state->halfmove_clock;
    // castling
    if (from_piece == &board->k)
    {
        state->fields &= is_white
            ? ~(BOARD_STATE_FIELDS_CASTLING_WK | BOARD_STATE_FIELDS_CASTLING_WQ)
            : ~(BOARD_STATE_FIELDS_CASTLING_BK | BOARD_STATE_FIELDS_CASTLING_BQ);
    }
    if ((move->from|move->to) & A1) state->fields &= ~BOARD_STATE_FIELDS_CASTLING_WQ;
    if ((move->from|move->to) & H1) state->fields &= ~BOARD_STATE_FIELDS_CASTLING_WK;
    if ((move->from|move->to) & A8) state->fields &= ~BOARD_STATE_FIELDS_CASTLING_BQ;
    if ((move->from|move->to) & H8) state->fields &= ~BOARD_STATE_FIELDS_CASTLING_BK;
    // en passant
    state->en_passant_square = 0;
    if (from_piece == &board->p && square_log2_diff(move->from, move->to) == 2 * RANK_SIZE)
    {
        const square_t opponent_pawns = board->p & (is_white ? ~board->w : board->w);
        if (((move->to    & ~(FILE_H)) && ((move->to << 1) & opponent_pawns))
            || ((move->to & ~(FILE_A)) && ((move->to >> 1) & opponent_pawns)))
        {
            state->en_passant_square = is_white
                ? move->to >> RANK_SIZE
                : move->to << RANK_SIZE;
        }
    }
    // check for king attacks
    return !board_state_get_attacked_kings(state, is_white)
        ? APPLY_MOVE_STATUS_OK
        : APPLY_MOVE_STATUS_ILLEGAL_ANY_KING_ATTACKED;
}

size_t board_state_get_legal_moves(const BoardState* state, Move* moves, size_t moves_size)
{
    assert(state != NULL);
    assert(moves != NULL);
    assert(moves_size >= BOARD_STATE_MOVES_SIZE);
    Move temp_moves[BOARD_STATE_MOVES_SIZE];
    const size_t length = board_state_get_pseudo_legal_moves(state, temp_moves, BOARD_STATE_MOVES_SIZE);
    BoardState copy;
    size_t idx = 0;
    for (size_t i=0; i<length; ++i)
    {
        board_state_copy(state, &copy);
        if (board_state_apply_move(&copy, &temp_moves[i]) == APPLY_MOVE_STATUS_OK)
            moves[idx++] = temp_moves[i];
    }
    return idx;
}

float board_state_evaluate_piece_count(const BoardState* state)
{
    assert(state != NULL);
    #define EVALUATE_PIECE_COUNT(scalar, piece) \
        scalar * (__builtin_popcountll(board->piece & board->w) - __builtin_popcountll(board->piece & ~board->w))
    const Bitboard* board = &state->board;
    return EVALUATE_PIECE_COUNT(200.0f, k)
        + EVALUATE_PIECE_COUNT(9.0f, q)
        + EVALUATE_PIECE_COUNT(5.0f, r)
        + EVALUATE_PIECE_COUNT(3.0f, b)
        + EVALUATE_PIECE_COUNT(3.0f, n)
        + EVALUATE_PIECE_COUNT(1.0f, p);
    #undef EVALUATE_PIECE_COUNT
}

evaluation_t board_state_evaluate(const BoardState* state)
{
    assert(state != NULL);
    return board_state_evaluate_piece_count(state);
}

evaluation_t board_state_evaluate_minimax(const BoardState* state, uint64_t depth)
{
    assert(state != NULL);
    const evaluation_t sign = board_state_is_white(state) ? 1.0f : -1.0f;
    if (depth == 0)
        return sign * board_state_evaluate(state);
    Move moves[BOARD_STATE_MOVES_SIZE];
    const size_t moves_size = board_state_get_pseudo_legal_moves(state, moves, BOARD_STATE_MOVES_SIZE);
    if (moves_size == 0)
        return -INFINITY;
    evaluation_t max_evaluation = -INFINITY;
    for (size_t i=0; i<moves_size; ++i)
    {
        const Move* move = &moves[i];
        BoardState copy  = {0};
        board_state_copy(state, &copy);
        board_state_apply_move(&copy, move);
        const evaluation_t evaluation = board_state_evaluate_minimax(&copy, depth - 1);
        if (evaluation > max_evaluation)
            max_evaluation = evaluation;
    }
    return max_evaluation;
}

void move_to_string(const Move* move, char* str, size_t str_size)
{
    assert(move != NULL);
    assert(str != NULL);
    assert(str_size >= MOVE_TO_STRING_SIZE);
    square_to_string(move->from, str, SQUARE_TO_STRING_SIZE);
    square_to_string(move->to, str + (SQUARE_TO_STRING_SIZE-1), SQUARE_TO_STRING_SIZE);
    char castling = ' ';
    if (move->fields & MOVE_FIELDS_QUEENING_CHOICE_Q)
        castling = 'q';
    else if (move->fields & MOVE_FIELDS_QUEENING_CHOICE_R)
        castling = 'r';
    else if (move->fields & MOVE_FIELDS_QUEENING_CHOICE_B)
        castling = 'b';
    else if (move->fields & MOVE_FIELDS_QUEENING_CHOICE_N)
        castling = 'n';
    str[4] = (castling != ' ') ? castling : '\0';
}

Move string_to_move(const char* str, size_t str_size)
{
    assert(str_size >= STRING_TO_MOVE_SIZE);
    const square_t from = string_to_square(str, STRING_TO_SQUARE_SIZE);
    const square_t to   = string_to_square(str + STRING_TO_SQUARE_SIZE, STRING_TO_SQUARE_SIZE);
    uint8_t fields = 0;
    if (str_size > STRING_TO_MOVE_SIZE)
    {
        switch (str[4])
        {
            case 'q': fields |= MOVE_FIELDS_QUEENING_CHOICE_Q; break;
            case 'r': fields |= MOVE_FIELDS_QUEENING_CHOICE_R; break;
            case 'b': fields |= MOVE_FIELDS_QUEENING_CHOICE_B; break;
            case 'n': fields |= MOVE_FIELDS_QUEENING_CHOICE_N; break;
            default: break;
        }
    }
    return (Move)
    {
        .from   = from,
        .to     = to,
        .fields = fields
    };
} 

