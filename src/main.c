#include <stdio.h>

#include "../include/game.h"
#include "../include/board.h"
#include "../include/player.h"

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    Game g;
    init_game(&g);
    while(!game_over(&g.board)){
        run_game(&g);
    }
    declare_winner(&g);
    return 0;
}
