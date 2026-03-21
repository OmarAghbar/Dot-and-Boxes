#ifndef BOARD_H
#define BOARD_H

#define ROWS 5
#define COLS 6

typedef struct {
    int h_lines[ROWS][COLS-1];
    int v_lines[ROWS-1][COLS];
    char boxes[ROWS-1][COLS-1];
} Board;

// Methods
void init_board(Board *b);
void display_board(Board *b); //will be called every time a player does a move
int can_draw(Board *b, int first_point, int second_point, int third_point, int fourth_point); 
int draw_line(Board *b, int first_point, int second_point, int third_point, int fourth_point);   // should return 1 if a line is drawn successfully, or 0 if you not
int check_boxes(Board *b, int first_point, int second_point, int third_point, int fourth_point, char player); // sing the player's symbol to mark the box if the player completes a box, and return the number of boxes completed by this move
int game_over(Board *b); //if all boxes are filled, then return 1, else return 0 and continue the game

#endif