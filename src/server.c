#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/board.h"
#include "../include/player.h"

#define DEFAULT_PORT 9090
#define BUF_SIZE     256

/* ============================================================
   Protocol (should be agreed upon by all teams, send only 4 ints):

   Server → Client:
     YOU_ARE A\n  or  YOU_ARE B\n    (once at connection)
     WAIT\n                          (not your turn)
     YOUR_TURN\n                     (go ahead)
     MOVE r1 c1 r2 c2\n              (relayed move from other player)
     GAMEOVER A\n / GAMEOVER B\n / GAMEOVER D\n

   Client → Server:
     MOVE r1 c1 r2 c2\n              (the move you made)
   ============================================================ */
typedef struct {
    Board           board;          /* server tracks the board itself   */
    Player          players[2];     /* server tracks scores itself      */
    pthread_mutex_t lock;
    pthread_cond_t  turn_cv;
    int             client_fds[2];
    int             current_turn;   /* 0 = A, 1 = B                     */
    int             done;
} ServerState;

typedef struct {
    ServerState *state;
    int          player_idx;
} ThreadArg;

static void send_str(int fd, const char *msg)
{
    send(fd, msg, strlen(msg), 0);
}

static void broadcast_gameover(ServerState *s)
{
    char buf[32];
    char winner;
    if      (s->players[0].score > s->players[1].score) winner = 'A';
    else if (s->players[1].score > s->players[0].score) winner = 'B';
    else                                                 winner = 'D';
    snprintf(buf, sizeof(buf), "GAMEOVER %c\n", winner);
    send_str(s->client_fds[0], buf);
    send_str(s->client_fds[1], buf);
}

/* ============================================================
   Client thread
   ============================================================ */
static void *client_handler(void *arg)
{
    ThreadArg   *ta    = (ThreadArg *)arg;
    ServerState *s     = ta->state;
    int          idx   = ta->player_idx;
    int          other = (idx + 1) % 2;
    int          fd    = s->client_fds[idx];
    free(ta);

    char buf[BUF_SIZE];

    while (1) {

        /* PHASE 1: wait for our turn */
        pthread_mutex_lock(&s->lock);

        while (s->current_turn != idx && !s->done)
            pthread_cond_wait(&s->turn_cv, &s->lock);

        if (s->done) {
            pthread_mutex_unlock(&s->lock);
            break;
        }

        send_str(fd, "Your Turn\n");
        pthread_mutex_unlock(&s->lock);

        /* PHASE 2: inner loop, keep turn while scoring */
        int keep_turn = 1;

        while (keep_turn) {

            /* recv with no lock held because the other thread sleeping in cond_wait */
            int bytes = recv(fd, buf, BUF_SIZE - 1, 0);

            if (bytes <= 0) {
                printf("[server] Player %c disconnected.\n",
                       idx == 0 ? 'A' : 'B');
                pthread_mutex_lock(&s->lock);
                s->done = 1;
                pthread_cond_broadcast(&s->turn_cv);
                pthread_mutex_unlock(&s->lock);
                goto thread_exit;
            }

            buf[bytes] = '\0';

            /* parse "MOVE r1 c1 r2 c2" */
            int r1, c1, r2, c2;
            if (sscanf(buf, "MOVE %d %d %d %d", &r1, &c1, &r2, &c2) != 4) {
                send_str(fd, "Invalid move format. Try again.\n");
                continue;
            }

            /* validate, apply, display (under the lock) */
            pthread_mutex_lock(&s->lock);

            if (!can_draw(&s->board, r1, c1, r2, c2)) {
                send_str(fd, "Invalid move. Try again.\n");
                pthread_mutex_unlock(&s->lock);
                continue;
            }

            /* apply the move to the server's own board */
            draw_line(&s->board, r1, c1, r2, c2);
            int scored = check_boxes(&s->board, r1, c1, r2, c2,
                                     s->players[idx].initial);
            increment_score(&s->players[idx], scored);

            /* display the updated board on the server terminal */
            printf("\n[server] Player %c played: %d %d -> %d %d\n",
                   s->players[idx].initial, r1, c1, r2, c2);
            display_board(&s->board);
            print_scores(&s->players[0], &s->players[1]);

            /* relay the raw move string to the other client */
            send_str(s->client_fds[other], buf);

            /* check game over */
            if (game_over(&s->board)) {
                broadcast_gameover(s);
                s->done = 1;
                pthread_cond_broadcast(&s->turn_cv);
                pthread_mutex_unlock(&s->lock);
                goto thread_exit;
            }

            if (scored > 0) {
                /* player scored, they keep their turn */
                send_str(fd, "Your turn again\n");
            } else {
                /* no score, switch turn and wake the other thread */
                s->current_turn = other;
                pthread_cond_broadcast(&s->turn_cv);
                keep_turn = 0;
            }

            pthread_mutex_unlock(&s->lock);

        }

    }

thread_exit:
    printf("[server] Player %c thread exiting.\n",
           s->players[idx].initial);
    return NULL;
}

int main(int argc, char *argv[])
{
    int port = DEFAULT_PORT;
    if (argc >= 2)
        port = atoi(argv[1]);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons(port)
    };
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(server_fd, 2) < 0) { perror("listen"); exit(1); }

    printf("[server] Listening on port %d. Waiting for 2 players...\n", port);

    /* initialize shared state */
    ServerState state;
    memset(&state, 0, sizeof(state));
    pthread_mutex_init(&state.lock, NULL);
    pthread_cond_init(&state.turn_cv, NULL);
    init_board(&state.board);
    init_player(&state.players[0], 'A');
    init_player(&state.players[1], 'B');
    state.current_turn = 0;
    state.done         = 0;

    /* accept exactly 2 clients */
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);

    for (int i = 0; i < 2; i++) {
        int fd = accept(server_fd, (struct sockaddr *)&cli_addr, &cli_len);
        if (fd < 0) { perror("accept"); exit(1); }

        state.client_fds[i] = fd;

        char msg[32];
        snprintf(msg, sizeof(msg), "You are %c\n", i == 0 ? 'A' : 'B');
        send_str(fd, msg);

        printf("[server] Player %c connected from %s\n",
               i == 0 ? 'A' : 'B', inet_ntoa(cli_addr.sin_addr));
    }

    printf("[server] Both players connected. Game starting!\n");
    display_board(&state.board);

    /* player B waits for A to go first */
    send_str(state.client_fds[1], "WAIT\n");

    /* spawn one thread per client */
    pthread_t threads[2];
    for (int i = 0; i < 2; i++) {
        ThreadArg *ta  = malloc(sizeof(ThreadArg));
        ta->state      = &state;
        ta->player_idx = i;
        if (pthread_create(&threads[i], NULL, client_handler, ta) != 0) {
            perror("pthread_create"); exit(1);
        }
    }

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    for (int i = 0; i < 2; i++) close(state.client_fds[i]);
    close(server_fd);
    pthread_mutex_destroy(&state.lock);
    pthread_cond_destroy(&state.turn_cv);

    printf("[server] Server shutting down.\n");
    return 0;
}