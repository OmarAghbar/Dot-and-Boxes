#ifndef PLAYER_H_
#define PLAYER_H_

typedef struct {
    char initial;   // 'A' or 'B'
    int score;
} Player;
void init_player(Player *p, char initial);
void increment_score(Player *p, int amount);
void print_scores(Player *a, Player *b);

#endif