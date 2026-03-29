#include <stdio.h>

#include "../include/player.h"

void init_player(Player *p, char initial){
    p->initial = initial;
    p->score = 0;
}

void increment_score(Player *p, int amount) {
    if(amount > 0){
        p->score += amount;
    }
}

void print_scores(Player *a, Player*b) {
    printf("***********************************************************\n");
    printf("Player %c score: %d\n", a->initial, a->score);
    printf("Player %c score: %d\n", b->initial, b->score);
    printf("***********************************************************\n\n");
}