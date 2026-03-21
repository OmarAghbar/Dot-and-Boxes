#ifndef GAME_H
#define GAME_H

typedef struct {
    Board board;
    Player players[2];
    int current_turn; //0 for the turn of player 1 and 1 for the turn of player 2
} Game;
void init_game(Game *g);
void run_game(Game *g);
void switch_turn(Game *g);
void declare_winner(Game *g);

#endif