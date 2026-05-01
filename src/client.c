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
#include "../include/client.h"

typedef struct {
    Board           board;
    Player          players[2];
    int             my_idx;     /* 0 = A, 1 = B */
    int             my_turn;    /* 1 when it is our turn to move */
    int             game_over;
    int             sockfd;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} ClientState;

/* Send a null-terminated string over the socket. */
static void send_str(int fd, const char *msg)
{
    send(fd, msg, strlen(msg), 0);
}

/* Read exactly one newline-terminated line from the socket (blocking).
   Stores the line without the '\n' into buf (max maxlen-1 chars + NUL). */
static int read_line(int fd, char *buf, int maxlen)
{
    int i = 0;
    char c;
    while (i < maxlen - 1) {
        int r = recv(fd, &c, 1, 0);
        if (r <= 0) return -1;
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

/* ---------------------------------------------------------------
   Receiver thread – reads all server messages, updates shared state
   --------------------------------------------------------------- */
static void *receiver_thread(void *arg)
{
    ClientState *cs = (ClientState *)arg;

    char netbuf[4096];
    int  netbuf_len = 0;

    while (1) {
        int n = recv(cs->sockfd,
                     netbuf + netbuf_len,
                     (int)sizeof(netbuf) - netbuf_len - 1,
                     0);
        if (n <= 0) {
            fprintf(stderr, "\n[client] Server connection closed.\n");
            pthread_mutex_lock(&cs->mutex);
            cs->game_over = 1;
            pthread_cond_signal(&cs->cond);
            pthread_mutex_unlock(&cs->mutex);
            break;
        }
        netbuf_len += n;
        netbuf[netbuf_len] = '\0';

        /* Process every complete line in the buffer. */
        char *start = netbuf;
        char *nl;
        while ((nl = strchr(start, '\n')) != NULL) {
            *nl = '\0';

            char line[512];
            strncpy(line, start, sizeof(line) - 1);
            line[sizeof(line) - 1] = '\0';
            start = nl + 1;

            /* --- YOUR_TURN ----------------------------------------- */
            if (strcmp(line, "YOUR_TURN") == 0) {
                pthread_mutex_lock(&cs->mutex);
                cs->my_turn = 1;
                pthread_cond_signal(&cs->cond);
                pthread_mutex_unlock(&cs->mutex);

            /* --- WAIT ---------------------------------------------- */
            } else if (strcmp(line, "WAIT") == 0) {
                printf("Waiting for your turn...\n");
                fflush(stdout);

            /* --- INVALID ------------------------------------------- */
            } else if (strcmp(line, "INVALID") == 0) {
                printf("Server rejected that move. Please try again.\n");
                fflush(stdout);

            /* --- MOVE r1 c1 r2 c2  (opponent's move) --------------- */
            } else if (strncmp(line, "MOVE ", 5) == 0) {
                int r1, c1, r2, c2;
                if (sscanf(line + 5, "%d %d %d %d", &r1, &c1, &r2, &c2) == 4) {
                    pthread_mutex_lock(&cs->mutex);

                    int opp = 1 - cs->my_idx;
                    draw_line(&cs->board, r1, c1, r2, c2);
                    int scored = check_boxes(&cs->board, r1, c1, r2, c2,
                                            cs->players[opp].initial);
                    increment_score(&cs->players[opp], scored);

                    printf("\nOpponent played: %d %d -> %d %d\n",
                           r1, c1, r2, c2);
                    display_board(&cs->board);
                    print_scores(&cs->players[0], &cs->players[1]);
                    fflush(stdout);

                    pthread_mutex_unlock(&cs->mutex);
                }

            /* --- GAMEOVER X ---------------------------------------- */
            } else if (strncmp(line, "GAMEOVER ", 9) == 0) {
                char winner = line[9];
                printf("\n========== GAME OVER ==========\n");
                if (winner == 'D') {
                    printf("It's a draw!\n");
                } else if (winner == cs->players[cs->my_idx].initial) {
                    printf("You win! Congratulations!\n");
                } else {
                    printf("You lose. Better luck next time.\n");
                }
                printf("================================\n");
                fflush(stdout);

                pthread_mutex_lock(&cs->mutex);
                cs->game_over = 1;
                pthread_cond_signal(&cs->cond);
                pthread_mutex_unlock(&cs->mutex);
            }
        }

        /* Shift leftover partial data to the front of the buffer. */
        int remaining = (int)(netbuf_len - (start - netbuf));
        memmove(netbuf, start, (size_t)remaining);
        netbuf_len = remaining;
    }

    return NULL;
}

/* ---------------------------------------------------------------
   run_client – entry point called from main.c
   --------------------------------------------------------------- */
int run_client(const char *server_ip, int port)
{
    /* 1. Create socket and connect. */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return -1; }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons((uint16_t)port);
    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address: %s\n", server_ip);
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }
    printf("Connected to server at %s:%d\n", server_ip, port);

    /* 2. Read the YOU_ARE message synchronously before spawning the thread. */
    char first_line[64];
    if (read_line(sockfd, first_line, (int)sizeof(first_line)) < 0) {
        fprintf(stderr, "Failed to read server greeting.\n");
        close(sockfd);
        return -1;
    }

    char my_initial = 0;
    if (sscanf(first_line, "YOU_ARE %c", &my_initial) != 1) {
        fprintf(stderr, "Unexpected greeting: %s\n", first_line);
        close(sockfd);
        return -1;
    }
    printf("You are Player %c\n", my_initial);

    /* 3. Initialize shared state. */
    ClientState cs;
    memset(&cs, 0, sizeof(cs));
    cs.sockfd   = sockfd;
    cs.my_idx   = (my_initial == 'A') ? 0 : 1;
    cs.my_turn  = 0;
    cs.game_over = 0;
    init_board(&cs.board);
    init_player(&cs.players[0], 'A');
    init_player(&cs.players[1], 'B');
    pthread_mutex_init(&cs.mutex, NULL);
    pthread_cond_init(&cs.cond, NULL);

    printf("\nInitial board:\n");
    display_board(&cs.board);
    print_scores(&cs.players[0], &cs.players[1]);
    fflush(stdout);

    /* 4. Spawn receiver thread. */
    pthread_t recv_tid;
    if (pthread_create(&recv_tid, NULL, receiver_thread, &cs) != 0) {
        perror("pthread_create");
        close(sockfd);
        return -1;
    }

    /* 5. Main (sender) loop. */
    while (1) {
        /* Wait until it is our turn or the game ends. */
        pthread_mutex_lock(&cs.mutex);
        while (!cs.my_turn && !cs.game_over)
            pthread_cond_wait(&cs.cond, &cs.mutex);

        if (cs.game_over) {
            pthread_mutex_unlock(&cs.mutex);
            break;
        }
        pthread_mutex_unlock(&cs.mutex);

        /* Prompt for a move (no lock held while scanning input). */
        int r1, c1, r2, c2;
        int valid_input = 0;
        while (!valid_input) {
            printf("Your turn! Enter move (r1 c1 r2 c2): ");
            fflush(stdout);

            if (scanf("%d %d %d %d", &r1, &c1, &r2, &c2) != 4) {
                /* Clear bad input */
                int ch;
                while ((ch = getchar()) != '\n' && ch != EOF);
                printf("Invalid input format. Use: r1 c1 r2 c2\n");
                continue;
            }

            /* Validate against the local board under the lock. */
            pthread_mutex_lock(&cs.mutex);
            if (!can_draw(&cs.board, r1, c1, r2, c2)) {
                printf("That line is already drawn or out of bounds. Try again.\n");
                fflush(stdout);
                pthread_mutex_unlock(&cs.mutex);
                continue;
            }

            /* Apply our own move to the local board. */
            draw_line(&cs.board, r1, c1, r2, c2);
            int scored = check_boxes(&cs.board, r1, c1, r2, c2,
                                     cs.players[cs.my_idx].initial);
            increment_score(&cs.players[cs.my_idx], scored);

            printf("\nYou played: %d %d -> %d %d\n", r1, c1, r2, c2);
            display_board(&cs.board);
            print_scores(&cs.players[0], &cs.players[1]);
            fflush(stdout);

            cs.my_turn = 0;
            pthread_mutex_unlock(&cs.mutex);
            valid_input = 1;
        }

        /* Send move to server. */
        char msg[64];
        snprintf(msg, sizeof(msg), "MOVE %d %d %d %d\n", r1, c1, r2, c2);
        send_str(sockfd, msg);
    }

    /* 6. Cleanup. */
    pthread_join(recv_tid, NULL);
    close(sockfd);
    pthread_mutex_destroy(&cs.mutex);
    pthread_cond_destroy(&cs.cond);

    return 0;
}
