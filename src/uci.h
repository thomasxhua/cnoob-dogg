#ifndef UCI_H
#define UCI_H

// Meyer-Kahlen, Huber, _UCI (=universal chess interface)
// https://www.stmintz.com/ccc/index.php?id=141612

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "board_state.h"
#include "dyn_array.h"
#include "simp_tree.h"

DEFINE_DYN_ARRAY(pthread_t, dyn_array_pthread)
DEFINE_SIMP_TREE(Move, simp_tree_move)

typedef enum
{
    UCI_SEARCH_MODE_STOP,
    UCI_SEARCH_MODE_DEPTH,
    UCI_SEARCH_MODE_INFINITE,
    UCI_SEARCH_MODE_PONDER,
} uci_search_mode_t;

typedef struct
{
    BoardState state;
    Move bestmove;
    uci_search_mode_t search_mode;
    uint64_t depth;
    bool debug_flag;
    dyn_array_pthread threads;
} UCIState;

#define UCI_BUFFER_SIZE 256
#define UCI_TOKENS_SIZE 64

#define UCI_CNOOBDOGG_NAME "cnoobdogg 0.1"
#define UCI_CNOOBDOGG_AUTHOR "Thomas Hua"

void uci_state_init(UCIState* uci);

void uci_run_dialog(void);

void uci_cmd_uci(void);
void uci_cmd_debug(UCIState* uci, char** tokens, size_t tokens_size);
void uci_cmd_isready(void);
void uci_cmd_position(UCIState* uci, char** tokens, size_t tokens_size);
void uci_cmd_go(UCIState* uci, char** tokens, size_t tokens_size);
void uci_cmd_stop(UCIState* uci);
void uci_cmd_quit(void);

void uci_send_id(void);
void uci_send_uciok(void);
void uci_send_readyok(void);
void uci_send_bestmove(UCIState* uci);

void* uci_search_loop(void* arg);

#endif // UCI_H

