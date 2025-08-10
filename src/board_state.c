#include "board_state.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

void board_state_to_fen_string(const BoardState* state, char* str, size_t str_size)
{
    assert(state != NULL);
    assert(str != NULL);
    assert(str_size >= BOARD_STATE_TO_FEN_STRING_SIZE);
    const Bitboard* board = &state->board;
    // piece placement data
    uint64_t empty_counter = 0;
    uint64_t file_num = FILE_SIZE;
    square_t square   = 1ULL << (file_num * RANK_SIZE);
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
            square = 1ULL << (--file_num * RANK_SIZE);
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

void board_state_copy(const BoardState* state, BoardState* other)
{
    assert(state != NULL);
    assert(other != NULL);
    memcpy(other, state, sizeof(BoardState));
}

square_t board_state_get_pseudo_legal_squares_pawns(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board = &state->board;
    const square_t all_pieces      = bitboard_get_all_pieces(board);
    const square_t opponent_pieces = all_pieces & (is_white ? ~board->w : board->w);
    const square_t own_pawns       = selection & board->p & (is_white ? board->w : ~board->w);
    const square_t moves_once      = is_white
        ? ~all_pieces & (own_pawns << RANK_SIZE)
        : ~all_pieces & (own_pawns >> RANK_SIZE);
    const square_t moves_twice     = is_white
        ? ~all_pieces & ((~all_pieces & ((own_pawns & RANK_2) << RANK_SIZE)) << RANK_SIZE)
        : ~all_pieces & ((~all_pieces & ((own_pawns & RANK_7) >> RANK_SIZE)) >> RANK_SIZE);
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
    return moves_once | moves_twice | attacks | state->en_passant_square;
}

square_t board_state_get_pseudo_legal_squares_knights(const BoardState* state, bool is_white, square_t selection)
{
    assert(state != NULL);
    const Bitboard* board = &state->board;
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
        | (own_knights & ~(FILE_A|FILE_B | RANK_1       )) >> ((1 * RANK_SIZE) - 2)   // ES
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
            copy.board.k &= C1|D1|E1;
            copy.board.w &= C1|D1|E1;
            if (!(all_pieces & (B1|C1|D1)) && !board_state_get_attacked_kings(&copy, is_white))
                moves |= C1;
        }
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_WK)
        {
            copy.board = state->board;
            copy.board.k &= E1|F1|G1;
            copy.board.w &= C1|D1|E1;
            if (!(all_pieces & (F1|G1)) && !board_state_get_attacked_kings(&copy, is_white))
                moves |= G1;
        }
    }
    else
    {
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_BQ)
        {
            copy.board = state->board;
            copy.board.k &= C8|D8|E8;
            if (!(all_pieces & (B8|C8|D8)) && !board_state_get_attacked_kings(&copy, !is_white))
                moves |= C8;
        }
        if (state->fields & BOARD_STATE_FIELDS_CASTLING_BK)
        {
            copy.board = state->board;
            copy.board.k &= E8|F8|G8;
            if (!(all_pieces & (F8|G8)) && !board_state_get_attacked_kings(&copy, !is_white))
                moves |= G8;
        }
    }
    return moves;
}

size_t board_state_get_pseudo_legal_moves_pawns(const BoardState* state, bool is_white, Move* moves, size_t moves_size)
{
    assert(state != NULL);
    assert(moves != NULL);
    assert(moves_size >= BOARD_STATE_MOVES_PIECES_SIZE);
    const Bitboard* board     = &state->board;
    const square_t own_pawns = board->p & (is_white ? board->w : ~board->w);
    size_t idx = 0;
    for (square_t from = 1ULL; from; from <<= 1)
    {
        if (!(from & own_pawns))
            continue;
        const square_t square_moves = board_state_get_pseudo_legal_squares_pawns(state, is_white, from);
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
    size_t board_state_get_pseudo_legal_moves_##name(const BoardState* state, bool is_white, Move* moves, size_t moves_size) \
    { \
        assert(state != NULL); \
        assert(moves != NULL); \
        assert(moves_size >= BOARD_STATE_MOVES_PIECES_SIZE); \
        const Bitboard* board     = &state->board; \
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

size_t board_state_get_pseudo_legal_moves(const BoardState* state, bool is_white, Move* moves, size_t moves_size)
{
    assert(state != NULL);
    assert(moves != NULL);
    assert(moves_size >= BOARD_STATE_MOVES_SIZE);
    size_t idx = 0;
    idx += board_state_get_pseudo_legal_moves_pawns(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_knights(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_bishops(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_rooks(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_queens(state, is_white, moves + idx, moves_size);
    idx += board_state_get_pseudo_legal_moves_kings(state, is_white, moves + idx, moves_size);
    return idx;
}

square_t board_state_get_attacked_kings(const BoardState* state, bool is_white)
{
    assert(state != NULL);
    const Bitboard* board = &state->board;
    const square_t moves_pawns   = board_state_get_pseudo_legal_squares_pawns(state, !is_white, BOARD_FULL);
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
    const bool is_white = state->fields & BOARD_STATE_FIELDS_ACTIVE_COLOR_W;
    bool is_capture = bitboard_get_piece_ptr(board, move->to);
    if (from_piece == &board->p)
    {

        square_t* queening_piece =
            (move->fields & MOVE_FIELDS_QUEENING_CHOICE_Q)   ? &board->q
            : (move->fields & MOVE_FIELDS_QUEENING_CHOICE_R) ? &board->r
            : (move->fields & MOVE_FIELDS_QUEENING_CHOICE_B) ? &board->b
            : (move->fields & MOVE_FIELDS_QUEENING_CHOICE_N) ? &board->n
            : &board->q;
        // handle en passant
        if (state->en_passant_square && (move->to == state->en_passant_square))
        {
            bitboard_clear_square(board, is_white
                ? (state->en_passant_square << RANK_SIZE)
                : (state->en_passant_square >> RANK_SIZE));
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
    else if (from_piece == &board->k && ((move->from < move->to)
            ? move->from << 2 == move->to
            : move->from >> 2 == move->to))
    {
        #define HANDLE_CASTLING(color, direction, clear_range_min, clear_range_max, king_square, rook_square) \
            if (color && (move->from direction move->to)) \
            { \
                for (square_t square = clear_range_min; square != clear_range_max << 1; square <<= 1) \
                    bitboard_clear_square(board, square); \
                bitboard_place_piece(board, king_square, &board->k, color); \
                bitboard_place_piece(board, rook_square, &board->r, color); \
            }
        HANDLE_CASTLING(is_white, >, A1, E1, C1, D1)       // WQ
        else HANDLE_CASTLING(is_white, <, E1, H1, G1, F1)  // WK
        else HANDLE_CASTLING(!is_white, >, A8, E8, C8, D8) // BQ
        else HANDLE_CASTLING(!is_white, <, E8, H8, G8, F8) // BK
        else bitboard_move_square(board, move->from, move->to);
        #undef HANDLE_CASTLING
    }
    // handle normal move
    else
    {
        bitboard_move_square(board, move->from, move->to);
    }
    // set board state data
    if (is_white)
        state->fullmove_count += 1;
    state->fields ^= BOARD_STATE_FIELDS_ACTIVE_COLOR_W;
    if (from_piece == &board->p || is_capture)
        state->halfmove_clock = 0;
    else
        ++state->halfmove_clock;
    if (from_piece == &board->k)
    {
        state->fields &= is_white
            ? ~(BOARD_STATE_FIELDS_CASTLING_WK | BOARD_STATE_FIELDS_CASTLING_WQ)
            : ~(BOARD_STATE_FIELDS_CASTLING_BK | BOARD_STATE_FIELDS_CASTLING_BQ);
    }
    if (from_piece == &board->r)
    {
        if (move->from == A1) state->fields &= ~BOARD_STATE_FIELDS_CASTLING_WQ;
        if (move->from == H1) state->fields &= ~BOARD_STATE_FIELDS_CASTLING_WK;
        if (move->from == A8) state->fields &= ~BOARD_STATE_FIELDS_CASTLING_BQ;
        if (move->from == H8) state->fields &= ~BOARD_STATE_FIELDS_CASTLING_BK;
    }
    // check for king attacks
    return !board_state_get_attacked_kings(state, is_white)
        ? APPLY_MOVE_STATUS_OK
        : APPLY_MOVE_STATUS_ILLEGAL_ANY_KING_ATTACKED;
}

size_t board_state_get_legal_moves(const BoardState* state, bool is_white, Move* moves, size_t moves_size)
{
    assert(state != NULL);
    assert(moves != NULL);
    assert(moves_size >= BOARD_STATE_MOVES_SIZE);
    Move temp_moves[BOARD_STATE_MOVES_SIZE];
    const size_t length = board_state_get_pseudo_legal_moves(state, is_white, temp_moves, BOARD_STATE_MOVES_SIZE);
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

