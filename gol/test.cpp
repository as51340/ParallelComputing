#include <iostream>
#include <math.h>
#include <mpi.h>

size_t ROWS, COLS;

char** create_board(char **board, int rows, int cols) {
    board = new char*[rows];
    for(int i = 0; i < rows; i++) board[i] = new char[cols];
    return board;
}

void free_board_memory(char **board, int rows) {
    for(int i = 0; i < rows; i++) delete board[i];
    delete board;
}


void update_board(char **board, int rows, int cols) {
    std::cout << "Array size: " << rows*cols << std::endl;
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            board[i][j] += 2;
        }
    }
}

void init_board(char **board, int rows, int cols) {
    uint16_t siz = sizeof(board);
    std::cout << "Array size: " << siz << std::endl;
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            board[i][j] = 0;
        }
    }
}

void print_board(char **board, int rows, int cols) {
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            std::cout << (int)board[i][j] << " ";
        }
        std::cout << std::endl;
    }
}


int main(int argc, char *argv[]) {

    if(argc < 3) {
        std::cout << "Ending program because not enough arguments were given: " << argc << std::endl;
        return -1;   
    }

    try
    {
        ROWS = atoi(argv[1]);
        COLS = atoi(argv[2]);
    }
    catch (std::exception const &exc)
    {
        std::cout << "One or more program arguments are invalid!" << std::endl;
        return 1;
    }

    std::cout << "ROWS " << ROWS << " COLS " << COLS;

    char **board = create_board(board, ROWS, COLS);
    init_board(board, ROWS, COLS);
    update_board(board, ROWS, COLS);
    print_board(board, ROWS, COLS);


    int numtasks, rank;
    // MPI_Init(&argc, &argv);
    // MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    // MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    printf("Using %d tasks!", numtasks);

    //TODO solve get this values
    const int NPROWS = sqrt(numtasks);  // number of processors is squared number
    const int NPCOLS = sqrt(numtasks);
    const int BLOCKROWS = ROWS / NPROWS;  // 
    const int BLOCKCOLS = COLS / NPCOLS;
    char **locMat = create_board(locMat, BLOCKROWS, BLOCKCOLS);
    init_board(locMat, BLOCKROWS, BLOCKCOLS);
    

    MPI_Datatype blocktype;
    MPI_Datatype blocktype2;

    MPI_Type_vector(BLOCKROWS, BLOCKCOLS, COLS, MPI_CHAR, &blocktype2);
    MPI_Type_create_resized( blocktype2, 0, sizeof(char), &blocktype);
    MPI_Type_commit(&blocktype);

    int disps[NPROWS*NPCOLS];
    int counts[NPROWS*NPCOLS];
    for(int i = 0; i < NPROWS; i++) {
        for(int j = 0; j < NPCOLS; j++) {
            disps[i*NPCOLS+j] = i*COLS*BLOCKROWS + j*BLOCKCOLS;
            counts[i*NPCOLS +j] = 1;
        }
    }

    MPI_Scatterv(board, counts, disps, blocktype, locMat, BLOCKROWS*BLOCKCOLS, MPI_CHAR, 0, MPI_COMM_WORLD);


    for(int proc = 0; proc < numtasks; proc++) {
        std::cout << "Local matrix" << std::endl;
        print_board(locMat, BLOCKROWS, BLOCKCOLS);
        std::cout << std::endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);


    free_board_memory(board, ROWS);
    free_board_memory(locMat, BLOCKROWS);

    return 0;
}