#include <limits.h>
#include "../include/bot.h"
#include "../include/board.h"

#define BOT_DEPTH 8
#define MAX_EDGES (ROWS * (COLS - 1) + (ROWS - 1) * COLS)

static int count_score(Board *b, char initial) {
    int score = 0;
    for (int i = 0; i < ROWS - 1; i++)
        for (int j = 0; j < COLS - 1; j++)
            if (b->boxes[i][j] == initial)
                score++;
    return score;
}

static int move_completes_box(Board *b, int r1, int c1, int r2, int c2) {
    if (r1 == r2) {
        int i = r1;
        int j = (c1 < c2) ? c1 : c2;
        if (i > 0 &&
            b->horizontal[i-1][j] && b->vertical[i-1][j] && b->vertical[i-1][j+1])
            return 1;
        if (i < ROWS-1 &&
            b->horizontal[i+1][j] && b->vertical[i][j] && b->vertical[i][j+1])
            return 1;
    } else {
        int i = (r1 < r2) ? r1 : r2;
        int j = c1;
        if (j > 0 &&
            b->vertical[i][j-1] && b->horizontal[i][j-1] && b->horizontal[i+1][j-1])
            return 1;
        if (j < COLS-1 &&
            b->vertical[i][j+1] && b->horizontal[i][j] && b->horizontal[i+1][j])
            return 1;
    }
    return 0;
}

static int get_all_moves(Board *b, Move *moves) {
    Move box_moves[MAX_EDGES];
    Move other_moves[MAX_EDGES];
    int box_count = 0, other_count = 0;

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS - 1; j++) {
            if (b->horizontal[i][j] == 0) {
                Move m = {i, j, i, j+1};
                if (move_completes_box(b, i, j, i, j+1))
                    box_moves[box_count++] = m;
                else
                    other_moves[other_count++] = m;
            }
        }
    }

    for (int i = 0; i < ROWS - 1; i++) {
        for (int j = 0; j < COLS; j++) {
            if (b->vertical[i][j] == 0) {
                Move m = {i, j, i+1, j};
                if (move_completes_box(b, i, j, i+1, j))
                    box_moves[box_count++] = m;
                else
                    other_moves[other_count++] = m;
            }
        }
    }

    for (int i = 0; i < box_count; i++)
        moves[i] = box_moves[i];
    for (int i = 0; i < other_count; i++)
        moves[box_count + i] = other_moves[i];

    return box_count + other_count;
}

static int minimax(Board b, int depth, int alpha, int beta, int maximizing,
                   char bot_initial, char opp_initial) {
    if (game_over(&b) || depth == 0)
        return count_score(&b, bot_initial) - count_score(&b, opp_initial);

    Move moves[MAX_EDGES];
    int total = get_all_moves(&b, moves);
    if (total == 0)
        return count_score(&b, bot_initial) - count_score(&b, opp_initial);

    char current = maximizing ? bot_initial : opp_initial;

    if (maximizing) {
        int best = INT_MIN;
        for (int i = 0; i < total; i++) {
            Board nb = b;
            draw_line(&nb, moves[i].r1, moves[i].c1, moves[i].r2, moves[i].c2);
            int scored = check_boxes(&nb, moves[i].r1, moves[i].c1,
                                     moves[i].r2, moves[i].c2, current);
            int next_max = (scored > 0 && !game_over(&nb)) ? 1 : 0;
            int val = minimax(nb, depth - 1, alpha, beta, next_max,
                              bot_initial, opp_initial);
            if (val > best) best = val;
            if (best > alpha) alpha = best;
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = INT_MAX;
        for (int i = 0; i < total; i++) {
            Board nb = b;
            draw_line(&nb, moves[i].r1, moves[i].c1, moves[i].r2, moves[i].c2);
            int scored = check_boxes(&nb, moves[i].r1, moves[i].c1,
                                     moves[i].r2, moves[i].c2, current);
            int next_max = (scored > 0 && !game_over(&nb)) ? 0 : 1;
            int val = minimax(nb, depth - 1, alpha, beta, next_max,
                              bot_initial, opp_initial);
            if (val < best) best = val;
            if (best < beta) beta = best;
            if (beta <= alpha) break;
        }
        return best;
    }
}

Move bot_get_best_move(Board *b, char bot_initial, char opp_initial) {
    Move moves[MAX_EDGES];
    int total = get_all_moves(b, moves);
    int max_depth;
    if      (total >= 35) max_depth = 5;
    else if (total >= 25) max_depth = 6;
    else                  max_depth = BOT_DEPTH;

    Move best_move = moves[0];
    for (int current_depth = 1; current_depth <= max_depth; current_depth++) {
        int alpha = INT_MIN;
        int beta = INT_MAX;
        int new_best_val = INT_MIN;
        Move new_best_move = moves[0];
        for (int i = 0; i < total; i++) {
            Board nb = *b;
            draw_line(&nb, moves[i].r1, moves[i].c1, moves[i].r2, moves[i].c2);
            int scored = check_boxes(&nb, moves[i].r1, moves[i].c1,
                                      moves[i].r2, moves[i].c2, bot_initial);
            int next_max = (scored > 0 && !game_over(&nb)) ? 1 : 0;
            int val = minimax(nb, current_depth - 1, alpha, beta, next_max,
                              bot_initial, opp_initial);
            if (val > new_best_val) {
                new_best_val = val;
                new_best_move = moves[i];
            }
            if (val > alpha) alpha = val;
        }
        best_move = new_best_move;
        for (int i = 1; i < total; i++) {
            if (moves[i].r1 == new_best_move.r1 && moves[i].c1 == new_best_move.c1 &&
            moves[i].r2 == new_best_move.r2 && moves[i].c2 == new_best_move.c2) {
            Move tmp = moves[0];
            moves[0] = moves[i];
            moves[i] = tmp;
            break;
            }   
        }
    }

    return best_move;
}
