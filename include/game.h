#ifndef GAME_H_
#define GAME_H_

#include "board.h"
#include "player.h"
#include "bot.h"

typedef struct {
    Board board;
    Player players[2];
    int current_turn; //0 for the turn of player 1 and 1 for the turn of player 2
    int bot_mode;     //1 if playing vs bot, 0 for two human players
} Game;
void init_game(Game *g);
void run_game(Game *g);
int play_turn(Game *g, Player *p);
int play_bot_turn(Game *g, Player *p);
void switch_turn(Game *g, Player **p);
void declare_winner(Game *g);

#endif