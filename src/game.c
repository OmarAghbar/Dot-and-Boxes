#include <stdio.h>

#include "../include/game.h"

void init_game(Game *g) {
    Board *b = &g->board;
    Player *playerA = &g->players[0];
    Player *playerB = &g->players[1];

    init_board(b);
    init_player(playerA, 'A');
    init_player(playerB, 'B');
    g->current_turn = 0;

    printf("Play vs bot? (1 = yes, 0 = two human players): ");
    int choice = 0;
    scanf("%d", &choice);
    g->bot_mode = (choice == 1) ? 1 : 0;
    if (g->bot_mode)
        printf("Bot mode enabled. You are player A, bot is player B.\n");
}

void run_game(Game *g) {
    Board *b = &g->board;
    Player *player = &g->players[g->current_turn];
    int scored;

    display_board(b);

    do {
        if (g->bot_mode && g->current_turn == 1)
            scored = play_bot_turn(g, player);
        else
            scored = play_turn(g, player);

        if (scored > 0) {
            display_board(b);
            print_scores(&g->players[0], &g->players[1]);
        }
    } while (!game_over(b) && scored > 0);

    if (!game_over(b))
        switch_turn(g, &player);
}

int play_turn(Game *g, Player *p) {
    Board *b = &g->board;
    int dots[4];

    printf("player %c's turn. Enter row and column of the first dot and second dot (e.g. 1 2 0 2): \n", p->initial);
    scanf("%d", &dots[0]);
    scanf("%d", &dots[1]);
    scanf("%d", &dots[2]);
    scanf("%d", &dots[3]);

    while(can_draw(b, dots[0], dots[1], dots[2], dots[3]) == 0) {
        while (getchar() != '\n') {
            printf("Invalid input. Enter 4 numbers: ");
        }
        printf("Invalid Input. Player %c please re-enter row and column of the first dot and second dot (e.g. 1 2 0 2): \n", p->initial);
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

int play_bot_turn(Game *g, Player *p) {
    Board *b = &g->board;
    char opp_initial = g->players[0].initial;

    printf("Bot (player %c) is thinking...\n", p->initial);
    Move m = bot_get_best_move(b, p->initial, opp_initial);
    printf("Bot plays: %d %d -> %d %d\n", m.r1, m.c1, m.r2, m.c2);

    draw_line(b, m.r1, m.c1, m.r2, m.c2);
    int score = check_boxes(b, m.r1, m.c1, m.r2, m.c2, p->initial);
    increment_score(p, score);
    return score;
}

void switch_turn(Game *g, Player **p) {
    g->current_turn = (g->current_turn + 1) % 2;
    *p = &g->players[g->current_turn];
}

void declare_winner(Game *g) {
    if(g->players[0].score > g->players[1].score){
        printf("Player %c is the winner!\n", g->players[0].initial);
    } else if (g->players[1].score > g->players[0].score){
        printf("Player %c is the winner!\n", g->players[1].initial);
    } else {
        printf("It's a draw!\n");
    }
}
