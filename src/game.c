#include <stdio.h>

#include "../include/game.h"
// #include "../include/board.h"
// #include "../include/player.h"

void init_game(Game *g) {
    Board *b = &g->board;
    Player *playerA = &g->players[0];
    Player *playerB = &g->players[1];

    init_board(b);
    init_player(playerA, 'A');
    init_player(playerB, 'B');
    g->current_turn = 0;
}

void run_game(Game *g){
    Board *b = &g->board;
    Player *player = &g->players[g->current_turn];

    display_board(b);
    while(game_over(b) != 1 && play_turn(g, player) != 0) {
        display_board(b);
        print_scores(&g->players[0], &g->players[1]);
    }
    switch_turn(g, &player);
}

int play_turn(Game *g, Player *p){
    Board *b = &g->board;
    int dots[4];

    printf("player %c's turn. Enter row and column of the first dot and second dot (e.g. 1 2 0 2): \n", p->initial);
    scanf("%d", &dots[0]);
    scanf("%d", &dots[1]);
    scanf("%d", &dots[2]);
    scanf("%d", &dots[3]);

    while(can_draw(b, dots[0], dots[1], dots[2], dots[3]) == 0) {
        printf("Invalid Input. Player %c please renter row and column of the first dot and second dot (e.g. 1 2 0 2): \n", p->initial);
        scanf("%d", &dots[0]);
        scanf("%d", &dots[1]);
        scanf("%d", &dots[2]);
        scanf("%d", &dots[3]);
    }

    draw_line(b, dots[0], dots[1], dots[2], dots[3]);
    int score = check_boxes(b, dots[0], dots[1], dots[2], dots[3], p->initial);
    increment_score(p, score);
    return score;
}

void switch_turn(Game *g, Player **p){
    g->current_turn = (g->current_turn + 1) % 2;
    *p = &g->players[g->current_turn];
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