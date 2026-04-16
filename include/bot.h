#ifndef BOT_H_
#define BOT_H_

#include "board.h"

typedef struct {
    int r1, c1, r2, c2;
} Move;

Move bot_get_best_move(Board *b, char bot_initial, char opp_initial);

#endif
