#ifndef PLAYER_H
#define PLAYER_H

typedef struct {
    char initial;   // 'A' or 'B'
    int score;
} Player;
void init_player(Player *p, char initial, int is_bot);
void increment_score(Player *p, int amount);
void print_scores(Player *a, Player *b);

#endif