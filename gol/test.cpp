#include "test.h"

void test1_init_board(char *board) {
    // ASSUMES 4*4 GRID
    board[0] = 0;
    board[1] = 0;
    board[2] = 0;
    board[3] = 1;
    board[4] = 0;
    board[5] = 0;
    board[6] = 0;
    board[7] = 0;
    board[8] = 1;
    board[9] = 1;
    board[10] = 1;
    board[11] = 1;
    board[12] = 0;
    board[13] = 1;
    board[14] = 0;
    board[15] = 0; 
}


void test2_init_board(char *board) {
    // ASSUMES 4*4 GRID
    board[0] = 1;
    board[1] = 0;
    board[2] = 0;
    board[3] = 0;
    board[4] = 0;
    board[5] = 0;
    board[6] = 0;
    board[7] = 1;
    board[8] = 0;
    board[9] = 1;
    board[10] = 1;
    board[11] = 0;
    board[12] = 1;
    board[13] = 0;
    board[14] = 0;
    board[15] = 1; 
}

void test3_init_board(char *board) { 
    // ASSUMES 4+4 GRID
    board[0] = 1;
    board[1] = 1;
    board[2] = 0;
    board[3] = 0;
    board[4] = 1;
    board[5] = 1;
    board[6] = 0;
    board[7] = 0;
    board[8] = 0;
    board[9] = 0;
    board[10] = 0;
    board[11] = 0;
    board[12] = 0;
    board[13] = 0;
    board[14] = 0;
    board[15] = 0;
}

void test3_serial_init_board(std::vector<std::vector<bool>> &board) {
    // ASSUMES 4*4 GRID
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            board[i][j] = false;
        }
    }
    board[0][0] = true;
    board[0][1] = true;
    board[1][0] = true;
    board[1][1] = true;
}

void test4_serial_init_board(std::vector<std::vector<bool>> &board) {
    // ASSUMES 4*4 GRID
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            board[i][j] = false;
        }
    }
    board[0][0] = true;
    board[0][1] = true;
    board[1][0] = true;
    board[1][1] = true;
    board[2][2] = true;
}

void test4_init_board(char *board) {
    for(int i = 0; i < 16; i++) {
        board[i] = 0;
    }
    board[0] = 1;
    board[1] = 1;
    board[4] = 1;
    board[5] = 1;
    board[10] = 1;  
}

void test5_init_board(char *board) {
    // ASSUMES 12*8 board
    for(int i = 0; i < 12; i++) {
        for(int j = 0; j < 8; j++) {
            board[i*8 + j] = 0;
        }
    }
    board[0] = 1;
    board[3] = 1;
    board[4] = 1;
    board[13] = 1;
    board[14] = 1;
    board[16] = 1;
    board[21] = 1;
    board[26] = 1;
    board[34] = 1;
    board[36] = 1;
    board[37] = 1; 
    board[41] = 1;
    board[46] = 1;
    board[49] = 1;
    board[51] = 1;
    board[52] = 1;
    board[54] = 1;
    board[55] = 1;
    board[57] = 1;
    board[58] = 1;
    board[62] = 1;
    board[63] = 1;
    board[65] = 1;
    board[68] = 1;
    board[69] = 1;
    board[71] = 1;
    board[73] = 1;
    board[76] = 1;
    board[80] = 1;
    board[82] = 1;
    board[84] = 1;
    board[85] = 1;
    board[87] = 1;
}