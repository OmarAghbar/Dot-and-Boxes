#include <stdio.h>

#include "../include/server.h"
#include "../include/game.h"
#include "../include/board.h"
#include "../include/player.h"
#include "../include/client.h"

int main() {
    printf("Dots and Boxes Game\n");
    printf("Do you want to play locally or online? (1 for local, 2 for online): ");
    int answer;
    scanf("%d", &answer);
    if(answer == 1){
        Game g;
        init_game(&g);
        while(!game_over(&g.board)){
            run_game(&g);
        }
        declare_winner(&g);
        return 0;
    } else if(answer == 2){
        printf("Do you want to host the game or join a game? (1 for host, 2 for join): ");
        int answer2;
        scanf("%d", &answer2);
        if(answer2 == 1){
            printf("Do you want to specify a port or use the default? (1 for yes, 2 for no): ");
            int answer3;
            scanf("%d", &answer3);
            if(answer3 == 1){
                printf("Enter the port number: ");
                int port;
                scanf("%d", &port);
                run_server(port);
            } else {
                run_server(0);
            }
        } else if(answer2 == 2){
            char ip[64];
            int port;
            printf("Enter server IP address: ");
            scanf("%63s", ip);
            printf("Enter server port: ");
            scanf("%d", &port);
            return run_client(ip, port);
        } else {
            printf("Invalid input. Exiting.\n");
            return 1;
        }
    }
}