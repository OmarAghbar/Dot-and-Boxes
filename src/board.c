#include <stdio.h>
#include <stdlib.h>
#include "../include/board.h"

void init_board(Board *b){
    for(int i = 0; i < ROWS;i++){
        for(int j = 0; j<COLS-1;j++){
            b->horizontal[i][j] = 0;
        }
    }
    for(int i = 0; i < ROWS-1;i++){
        for(int j = 0; j<COLS;j++){
            b->vertical[i][j] = 0;
        }
    }
    for(int i = 0; i < ROWS-1;i++){
        for(int j = 0; j<COLS-1;j++){
            b->boxes[i][j] = ' ';
        }
    }
    b->boxes_filled =0;
}   


void display_board(Board *b) {
    // Column header
    printf("  ");
    for (int j = 0; j < COLS; j++) {
        printf("%d", j);
        if (j < COLS - 1) printf(" ");
    }
    printf("\n");

    for (int i = 0; i < ROWS; i++) {
        printf("%d ", i);
        for (int j = 0; j < COLS; j++) {
            printf(".");
            if (j < COLS - 1)
                printf("%s", b->horizontal[i][j]==1 ? "-" : " ");
        }
        printf("\n");

        if (i < ROWS - 1) {
            printf("  ");
            printf("  ");
            for (int j = 0; j < COLS; j++) {
                printf("%s", b->vertical[i][j]==1 ? "|" : " ");
                if (j < COLS - 1)
                    printf("%c", b->boxes[i][j]);
            }
            printf("\n");
        }
    }
}

int check_if_horizontal_or_vertical(int p1, int p2, int p3, int p4) {
    if (p1 == p3 && abs(p4 - p2) == 1)
        return 1;
    if (p2 == p4 && abs(p3 - p1) == 1)
        return 2;
    return 0;
}

int can_draw(Board *b, int first_point, int second_point, int third_point, int fourth_point) {
    if (first_point < 0 || second_point < 0 || third_point < 0 || fourth_point < 0 ||
        first_point >= ROWS || second_point >= COLS || third_point >= ROWS || fourth_point >= COLS)
        return 0;

    int check = check_if_horizontal_or_vertical(first_point, second_point, third_point, fourth_point);

    if (check == 1) {
        int first_col = (second_point < fourth_point) ? second_point : fourth_point;
        if (b->horizontal[first_point][first_col] == 0)
            return 1;
    }
    else if (check == 2) {
        int first_row = (first_point < third_point) ? first_point : third_point;
        if (b->vertical[first_row][second_point] == 0)
            return 2;
    }

    return 0;
}

int draw_line(Board *b, int first_point, int second_point, int third_point, int fourth_point){
    int check = can_draw(b, first_point,  second_point, third_point, fourth_point);
    if(check == 1){
        int first_col = (second_point < fourth_point) ? second_point : fourth_point;
        b->horizontal[first_point][first_col] = 1;
        return 1;
    }    
    else if(check == 2){
        int first_row = (first_point < third_point) ? first_point : third_point;
        b->vertical[first_row][second_point] =1;
        return 1;
    }
    else{
        return 0;
    }
}

int check_boxes(Board *b, int first_point, int second_point, int third_point, int fourth_point, char player){
    int number = 0;
    int check = check_if_horizontal_or_vertical(first_point, second_point, third_point, fourth_point);
    if(check == 1){//means line drawn was horizontal
        int first_col = (second_point < fourth_point)? second_point:fourth_point;
        if(first_point-1 >= 0){//check three things: horizontal above it, vertical to left and vertical to right (from above)
            if(b->horizontal[first_point-1][first_col] && 
                b->vertical[first_point-1][first_col] && 
                b->vertical[first_point-1][first_col+1]){
                number++;
                b->boxes_filled++;
                b->boxes[first_point-1][first_col] = player;
            }
        }
        if(first_point+1 < ROWS){//same but check lines from below
            if(b->horizontal[first_point+1][first_col] && 
            b->vertical[first_point][first_col] && 
            b->vertical[first_point][first_col+1]){
                number++;
                b->boxes_filled++;
                b->boxes[first_point][first_col] = player;
            }
        }   
    }
    else if(check == 2){//means line drawn was vertical
        int first_row = (first_point < third_point) ? first_point : third_point;
        
        if(second_point-1 >= 0){//check box to the left: vertical to left, horizontal above and horizontal below
            if(b->vertical[first_row][second_point-1] && 
            b->horizontal[first_row][second_point-1] && 
            b->horizontal[first_row+1][second_point-1]){
                number++;
                b->boxes_filled++;
                b->boxes[first_row][second_point-1] = player;
            }
        }
        
        if(second_point < COLS-1){//same as above but to the right
            if(b->vertical[first_row][second_point+1] && 
            b->horizontal[first_row][second_point] && 
            b->horizontal[first_row+1][second_point]){
                number++;
                b->boxes_filled++;
                b->boxes[first_row][second_point] = player;
            }
        }
    }
    else{
        return 0;
    }

    return number;
}

int game_over(Board *b){
    return (b->boxes_filled == (ROWS-1)*(COLS-1));
}
