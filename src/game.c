#include <stdio.h>

#include "game.h"
#include "board.h"
#include "player.h"

void init_game(Game *g) {
    Board *b = g->board;
    Player *playerA = g->players[0];
    Player *playerB = g->players[1];

    init_board(b);
    init_player(playerA, playerA->initial, 0);
    init_player(playerB, playerB->initial, 0);
}

void run_game(Game *g){
    Board *b = g->board;
    Player *player = g->players[0];

    display_board(b);
    play_turn(player, g);
    print_scores(g->players[0], g->players[1]);

    switch_turn(g, &player);

    display_board(b);
    play_turn(player, g);
    print_scores(g->players[0], g->players[1]);

}

void play_turn(Game *g, Player *p){
    Board *b = g->board;
    int dots[4];
    do {
        printf("player %c's turn. Enter row and column of the first dot and second dot (e.g. 1 2 0 2): \n", p->initial);
        scanf("%d", dots[0]);
        scanf("%d", dots[1]);
        scanf("%d", dots[2]);
        scanf("%d", dots[3]);

    }while(!can_draw(b, dots[0], dots[1], dots[2], dots[3]));
    draw_line(b, dots[0], dots[1], dots[2], dots[3]);
    int score = check_boxes(b, dots[0], dots[1], dots[2], dots[3], p->initial);
    increment_score(p, score);
}

void switch_turn(Game *g, Player **p){
    g->current_turn = (g->current_turn + 1) % 2;
    *p = g->players[g->current_turn];
}

void declare_winner(Game *g){
    if(g->players[0].score > g->players[1].score){
        printf("Player %c is the winner!\n", g->players[0].initial);
    } else if (g->players[1].score > g->players[0].score){
        printf("Player %c is the winner!\n", g->players[1].initial);
    } else {
        printf("It's a draw!\n");
    }
}