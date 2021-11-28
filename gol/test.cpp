#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <mpi.h>
#include <ctime>
#include <vector>

int ROWS, COLS;
int BLOCKROWS, BLOCKCOLS;
int NPROWS, NPCOLS;
int NUMTASKS;

int upTag = 0, rightTag = 1, downTag = 2, leftTag = 3; // from sending perspective

int const globalBufferLength = 50;

void init_board(char *board, int rows, int cols) {
    uint16_t siz = sizeof(board);
    std::cout << "Array size: " << siz << std::endl;
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            board[i*cols + j] = (char)i*cols + j;
        }
    }
}

void initializeBoard(char* board, int rows, int cols)
{
    int deadCellMultiplyer = 2;
    srand(time(0));
    for (int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            board[i*cols+j] = (rand() % (deadCellMultiplyer + 1) == 0);
        }
    }
}

void print_board(char *board, int rows, int cols) {
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            printf("%3d ", (char)board[i*cols + j]);
        }
        std::cout << std::endl;
    }
}

void writeBoardToFile(char *board, size_t firstRow, size_t lastRow, size_t firstCol, size_t lastCol, std::string fileName, int iteration, uint processID, int rows, int cols)
{
    //Open file
    std::ofstream outputFile(fileName + "_" + std::to_string(iteration) + "_" + std::to_string(processID) + ".gol");
    //Write metadata
    outputFile << std::to_string(firstRow) << " " << std::to_string(lastRow) << std::endl;
    outputFile << std::to_string(firstCol) << " " << std::to_string(lastCol) << std::endl;
    //Write data
    for(size_t i = 0; i < rows; i++) {
        for(size_t j = 0; j < cols; j++) {
            outputFile << ((int)board[i*cols + j]) << "\t";
        }
        outputFile << std::endl;
    }
    //Close file
    outputFile.close();
}

std::string setUpProgram(size_t rows, size_t cols, int iteration_gap, int iterations, int processes)
{
    //Generate progam name based on current time, all threads should use the same name!
    time_t rawtime;
    struct tm *timeInfo;
    char buffer[globalBufferLength];
    time(&rawtime);
    timeInfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H-%M-%S", timeInfo);
    std::string programName(buffer);

    //Generate main file
    std::ofstream outputFile(programName + ".gol");
    outputFile << std::to_string(rows) << " " << std::to_string(cols) << " " << std::to_string(iteration_gap) << " " << std::to_string(iterations) << " " << std::to_string(processes) << std::endl;
    outputFile.close();

    return programName;
}

void updateBoard(char *board, int rank, MPI_Datatype col_type)
{
    // Number of rows in this board
    // Number of cols in this board
    // Current rank of processor
    // Datatype for calculating column vector
    // Pointer to the beginning of the board

    // Do asynchronus communication
    MPI_Request reqs[4];  // from the sending perspective
    MPI_Status stats[4]; // from the sending perspective

    MPI_Request recvReqs[4];
    MPI_Status recvStats[4];
    
    char **comm = new char*[4];
    for(int i = 0; i < 4; i++) comm[i] = nullptr;


    // FIRST SEND EVERYTHING
    // SEND UP
    size_t dest = 0;
    if(firstRow > 0) {
        dest = rank - NPCOLS;
    } else {
        dest = NPCOLS * (NPROWS - 1) + rank % NPCOLS;
    }
    std::cout << "Proc: " << rank << " is trying to send up" << std::endl;
    MPI_Isend(&board[0], BLOCKCOLS, MPI_CHAR, dest, upTag, MPI_COMM_WORLD, &reqs[upTag]); // send up

    // SEND DOWN
    if(lastRow < ROWS - 1) {
        dest = rank + NPCOLS;
    } else {
        dest = rank % NPCOLS;
    }
    std::cout << "Proc: " << rank << " is trying to send down" << std::endl;
    MPI_Isend(&board[BLOCKCOLS*(BLOCKROWS-1)], BLOCKCOLS, MPI_CHAR, dest, downTag, MPI_COMM_WORLD, &reqs[downTag]);  // send down

    // SEND LEFT
    if(firstCol > 0) {
        dest = rank - 1;
    } else {
        dest = rank + NPCOLS - 1;
    }
    std::cout << "Proc: " << rank << " is trying to send left" << std::endl;
    MPI_Isend(&(board[0]), 1, col_type, dest, leftTag, MPI_COMM_WORLD, &reqs[leftTag]); // send left

    // SEND RIGHT
    if(lastCol < COLS -1) {
        dest = rank + 1;
    } else {
        dest = rank - NPCOLS + 1;
    }
    std::cout << "Proc: " << rank << " is trying to send right" << std::endl;
    MPI_Isend(&(board[BLOCKCOLS-1]), 1, col_type, dest, rightTag, MPI_COMM_WORLD, &reqs[rightTag]); // send right
     
    // AFTER THAT SEND REQUESTS FOR RECEIVING
    // SEND RECEIVE FROM UP
    size_t src = 0;
    if(firstRow > 0) {
        src = rank - NPCOLS;
    } else {
        src = NPCOLS * (NPROWS - 1) + rank % NPCOLS;
    }
    comm[upTag] = new char[BLOCKCOLS];
    std::cout << "Proc: " << rank << " is trying to receive from up" << std::endl;
    MPI_Irecv(&comm[upTag][0], BLOCKCOLS, MPI_CHAR, src, downTag, MPI_COMM_WORLD, &recvReqs[upTag]); // receive from up

    // SEND RECEIVE FROM DOWN
    if(lastRow < ROWS - 1) {
        src = rank + NPCOLS;
    } else {
        src = rank % NPCOLS;
    }
    comm[downTag] = new char[BLOCKCOLS];
    MPI_Irecv(&comm[downTag][0], BLOCKCOLS, MPI_CHAR, src, upTag, MPI_COMM_WORLD, &recvReqs[downTag]); // receive from down

    // SEND RECEIVE FROM LEFT
    if(firstCol > 0) {
        src = rank - 1;
    } else {
        src = rank + NPCOLS - 1;
    }
    comm[leftTag] = new char[BLOCKROWS];
    MPI_Irecv(&(comm[leftTag][0]), BLOCKROWS, MPI_CHAR, src, rightTag, MPI_COMM_WORLD, &recvReqs[leftTag]);


    // SEND RECEIVE FROM RIGHT
    if(lastCol < COLS -1) {
        src = rank + 1;
    } else {
        src = rank - NPCOLS + 1;
    }
    comm[rightTag] = new char[BLOCKROWS]; 
    MPI_Irecv(&(comm[rightTag][0]), BLOCKROWS, MPI_CHAR, src, leftTag, MPI_COMM_WORLD, &recvReqs[rightTag]);


    // NOW PERFORM LIVE NEIGHBOURS UPDATE FROM VALUES YOU KNOW
    std::vector<std::vector<int>> liveNeighbors(BLOCKROWS, std::vector<int>(BLOCKCOLS, 0));
    //Count live neighbors
    for (size_t i = 0; i < BLOCKROWS; ++i)
    {
        for (size_t j = 0; j < BLOCKCOLS; ++j)
        {
            if (((char)board[i*BLOCKCOLS +j]) == 1)
            {
                if(i > 0) {
                    liveNeighbors[i-1][j]++; // update up neighbours
                }
                if(i < BLOCKROWS - 1) {
                    liveNeighbors[i+1][j]++;
                }
                if(j > 0) {
                    liveNeighbors[i][j-1]++;
                }
                if(j < BLOCKCOLS - 1) {
                    liveNeighbors[i][j+1]++;
                }
            }
        }
    }

    // NOW PERFORM UPDATE OF THE PART OF THE BOARD YOU KNOW HOW TO SOLVE
    //Update board
    for (size_t i = 1; i < BLOCKROWS-1; ++i) {
        for (size_t j = 1; j < BLOCKCOLS-1; ++j) {
            board[i*BLOCKCOLS + j] = (char)((liveNeighbors[i][j] == 3) || ((char)board[i*BLOCKCOLS +j]) == 1 && liveNeighbors[i][j] == 2));
        }
    }

    // NOW WAIT FOR NEIGHBOURS VECTOR SO YOU CAN PERFORM FINAL UPDATE
    // WAIT FOR VERTICAL COMMUNICATION
    MPI_Wait(&recvReqs[upTag], &recvStats[upTag]);
    MPI_Wait(&recvReqs[downTag], &recvStats[downTag]);
    for(int j = 0; j < BLOCKCOLS; j++) {
        // UPDATE FIRST ROW
        board[j] = (char)((liveNeighbors[0][j] + comm[upTag][j] == 3) || ((char)board[j]) == 1 && (liveNeighbors[0][j] + comm[upTag][j]) == 2));
        // UPDATE LAST ROW
        size_t lastRow = BLOCKROWS - 1;
        board[lastRow*BLOCKCOLS + j] = (char)((liveNeighbors[lastRow][j] + comm[downTag][j] == 3) || ((char)board[lastRow*BLOCKCOLS + j]) == 1 && (liveNeighbors[lastRow][j] + comm[downTag][j]) == 2));
    }

    // WAIT FOR HORIZONTAL COMMUNICATION
    MPI_Wait(&recvReqs[rightTag], &recvStats[rightTag]);
    MPI_Wait(&recvReqs[leftTag], &recvStats[leftTag]);
    for(int i = 0; i < BLOCKROWS; i++) {
        // TDOOD
    }






}


// Will return pointers to the beginnning of data
char** do_communication(char *board, int firstRow, int lastRow, int firstCol, int lastCol, int rank, MPI_Datatype col_type) {
    // Send up, down - receive up, down
    // Board is here localBoard
    MPI_Request reqs[4];  // from the sending perspective
    MPI_Status stats[4]; // from the sending perspective

    MPI_Request recvReqs[4];
    MPI_Status recvStats[4];
    
    char **comm = new char*[4];
    for(int i = 0; i < 4; i++) comm[i] = nullptr;
    


    // FIRST SEND EVERYTHING

    if(firstRow > 0) { // then I can send up - this means it is not in first row
        std::cout << "Proc: " << rank << " is trying to send up" << std::endl;
        MPI_Isend(&board[0], BLOCKCOLS, MPI_CHAR, rank-NPCOLS, upTag, MPI_COMM_WORLD, &reqs[upTag]); // send up
    }
    if(lastRow < ROWS - 1) { // If am not at the bottom then I can send down but also receive from down
        std::cout << "Proc: " << rank << " is trying to send down" << std::endl;
        MPI_Isend(&board[BLOCKCOLS*(BLOCKROWS-1)], BLOCKCOLS, MPI_CHAR, rank+NPCOLS, downTag, MPI_COMM_WORLD, &reqs[downTag]);  // send down
    }

    if(firstCol > 0) {
        std::cout << "Proc: " << rank << " is trying to send left" << std::endl;
        MPI_Isend(&(board[0]), 1, col_type, rank-1, leftTag, MPI_COMM_WORLD, &reqs[leftTag]); // send left
    }

    if(lastCol < COLS -1) {
        std::cout << "Proc: " << rank << " is trying to send right" << std::endl;
        MPI_Isend(&(board[BLOCKCOLS-1]), 1, col_type, rank+1, rightTag, MPI_COMM_WORLD, &reqs[rightTag]); // send right
    }
     
     //Now try to receive

    if(firstRow > 0) {
        comm[upTag] = new char[BLOCKCOLS];
        std::cout << "Proc: " << rank << " is trying to receive from up" << std::endl;
        MPI_Irecv(&comm[upTag][0], BLOCKCOLS, MPI_CHAR, rank-NPCOLS, downTag, MPI_COMM_WORLD, &recvReqs[upTag]); // receive from up
        std::cout << "Proc: " << rank << " received from up processor following elements" << std::endl;
        std::cout << "----------" << std::endl;
        for(int i = 0; i < BLOCKCOLS; i++) {
            printf("%3d ", (char)comm[upTag][i]);   
        }
        std::cout << std::endl;
        std::cout << "----------" << std::endl;
    }

    if(lastRow < ROWS - 1) {
        comm[downTag] = new char[BLOCKCOLS];
        std::cout << "Proc: " << rank << " is trying to receive from down" << std::endl;
        MPI_Irecv(&comm[downTag][0], BLOCKCOLS, MPI_CHAR, rank+NPCOLS, upTag, MPI_COMM_WORLD, &recvReqs[downTag]); // receive from down
        std::cout << "Proc: " << rank << " received from down processor following elements" << std::endl;
        std::cout << "----------" << std::endl;
        for(int i = 0; i < BLOCKCOLS; i++) {
            printf("%3d ", (char)comm[downTag][i]);   
        }
        std::cout << std::endl;
        std::cout << "----------" << std::endl;
    }

    if(firstCol > 0 ) { // then you can send to left
        comm[leftTag] = new char[BLOCKROWS];
        std::cout << "Proc: " << rank << " is trying to receive from left" << std::endl;
        MPI_Irecv(&(comm[leftTag][0]), BLOCKROWS, MPI_CHAR, rank-1, rightTag, MPI_COMM_WORLD, &recvReqs[leftTag]);
        std::cout << "Proc: " << rank << " received from left processor following elements" << std::endl;
        std::cout << "----------" << std::endl;
        for(int i = 0; i < BLOCKROWS; i++) {
            printf("%3d ", (char)comm[leftTag][i]);   
        }
        std::cout << std::endl;
        std::cout << "----------" << std::endl;
    }

    if(lastCol < COLS - 1) { // then you can send to right processor
        comm[rightTag] = new char[BLOCKROWS]; 
        std::cout << "Proc: " << rank << " is trying to receive from right" << std::endl;
        MPI_Irecv(&(comm[rightTag][0]), BLOCKROWS, MPI_CHAR, rank+1, leftTag, MPI_COMM_WORLD, &recvReqs[rightTag]);
        std::cout << "Proc: " << rank << " received from right processor following elements" << std::endl;
        std::cout << "----------" << std::endl;
        for(int i = 0; i < BLOCKROWS; i++) {
            printf("%3d ", (char)comm[rightTag][i]);   
        }
        std::cout << std::endl;
        std::cout << "----------" << std::endl;  
    }

    for(int i = 0; i < 4; i++) {
        if(comm[i] != nullptr) {
            //MPI_Wait(&reqs[i], &stats[i]);
            MPI_Wait(&recvReqs[i], &recvStats[i]);
      }
    }

    MPI_Type_free(&col_type);

    return comm;
}


int main(int argc, char *argv[]) {

    if (argc != 5)
    {
        std::cout << "This program should be called with four arguments! \nThese should be, the total number of rows; the total number of columns; the gap between saved iterations and the total number of iterations, in that order." << std::endl;
        return 1;
    }

    int iteration_gap, iterations;

    try
    {
        ROWS = atoi(argv[1]);
        COLS = atoi(argv[2]);
        iteration_gap = atoi(argv[3]);
        iterations = atoi(argv[4]);
    }
    catch (std::exception const &exc)
    {
        std::cout << "One or more program arguments are invalid!" << std::endl;
        return 1;
    }
    // Setup global board

    // INIT MPI
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &NUMTASKS);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // get partition values
    NPROWS = sqrt(NUMTASKS);  // number of processors is squared number
    NPCOLS = sqrt(NUMTASKS);
    BLOCKROWS = ROWS / NPROWS;  // 
    BLOCKCOLS = COLS / NPCOLS;

    // Board and local board
    char board[ROWS][COLS];
    char locBoard[BLOCKROWS][BLOCKCOLS];

    // Name of the program - all threads will use same program
    std::string programName;
    
    MPI_Status Stat;
    int firstRow, lastRow, firstCol, lastCol;

    size_t rowTag = 0, colTag = 1;

    if(rank == 0) {
        std::cout << "Number of processors: " << numtasks << std::endl;
        std::cout << "NPROWS: " << NPROWS << std::endl;
        std::cout << "NPCOLS: " << NPCOLS << std::endl;
        std::cout << "BlOCKROWS: " << BLOCKROWS << std::endl;
        std::cout << "BLOCKCOLS: " << BLOCKCOLS << std::endl;

        //init_board(&board[0][0], ROWS, COLS);
        init_board(&board[0][0], ROWS, COLS);
    
        std::cout << "Global matrix" << std::endl;
        print_board(&board[0][0], ROWS, COLS);
        std::cout << std::endl;

        // Only one program name
        programName = setUpProgram(ROWS, COLS, iteration_gap, iterations, numtasks);

        // Send offsets from processor 0 to all other processors
        for(size_t i = 0; i < NPROWS; i++) {
            for(size_t j = 0; j < NPCOLS; j++) {
                int locFirstRow = BLOCKROWS * i;
                int locFirstCol = BLOCKCOLS * j;
                int procNum = i*NPCOLS + j;
                MPI_Send(&locFirstRow, 1, MPI_INT, procNum, rowTag, MPI_COMM_WORLD);
                MPI_Send(&locFirstCol, 1, MPI_INT, procNum, colTag, MPI_COMM_WORLD);
            }
        }
    }

    // Receive first row variable and first col variable
    MPI_Recv(&firstRow, 1, MPI_INT, 0, rowTag, MPI_COMM_WORLD, &Stat);
    MPI_Recv(&firstCol, 1, MPI_INT, 0, colTag, MPI_COMM_WORLD, &Stat);

    lastRow = firstRow + BLOCKROWS - 1;
    lastCol = firstCol + BLOCKCOLS - 1;

    MPI_Datatype blocktype;
    MPI_Datatype blocktype2;
    MPI_Datatype col_type;

    MPI_Type_vector(BLOCKROWS, BLOCKCOLS, COLS, MPI_CHAR, &blocktype2);
    MPI_Type_create_resized(blocktype2, 0, sizeof(char), &blocktype);
    MPI_Type_commit(&blocktype);
     // number of elements, blocklen, stride
    MPI_Type_vector(BLOCKROWS, 1, BLOCKCOLS, MPI_CHAR, &col_type);
    MPI_Type_commit(&col_type);


    int disps[NPROWS*NPCOLS];
    int counts[NPROWS*NPCOLS];

    for(int i = 0; i < NPROWS; i++) {
        for(int j = 0; j < NPCOLS; j++) {
            disps[i*NPCOLS+j] = i*COLS*BLOCKROWS + j*BLOCKCOLS;
            counts[i*NPCOLS +j] = 1;
        }
    }


    MPI_Scatterv(board, counts, disps, blocktype, locBoard, BLOCKROWS*BLOCKCOLS, MPI_CHAR, 0, MPI_COMM_WORLD);  

    std::cout << "Proc: " << rank << " First row: " << firstRow << " Last row: " << lastRow << " First col: " << firstCol << " Last col: " << lastCol << std::endl;
    print_board(&locBoard[0][0], BLOCKROWS, BLOCKCOLS);
    std::cout << std::endl;


    char **upDown = do_communication(&locBoard[0][0], firstRow, lastRow, firstCol, lastCol, rank, col_type);
    
    // std::cout << "Proc: " << rank << " returned from do communication" << std::endl;
    // for(int i = 0; i < 4; i++) {
    //     if(upDown[i] != nullptr) {
    //         std::cout << "Printing what was received from tag " << i << std::endl;
    //         std::cout << ".........." << std::endl;
    //         for(int j = 0; j < BLOCKCOLS; j++) {
    //             printf("%3d ", (int)upDown[i][j]);   
    //         }
    //         std::cout << std::endl << ".........." << std::endl;
    //     }
    // }


    //for(int i = 0; i < 4; i++) {
    //    delete upDown[i];
    //}
//
    //delete upDown;
    
    
    // //Do iteration
    writeBoardToFile(&locBoard[0][0], firstRow, lastRow, firstCol, lastCol, programName, 0, rank, BLOCKROWS, BLOCKCOLS);
    for (int i = 1; i <= iterations; ++i)
    {
        updateBoard(&locBoard[0][0], BLOCKROWS, BLOCKCOLS);
        if (i % iteration_gap == 0)
        {
            writeBoardToFile(&locBoard[0][0], firstRow, lastRow, firstCol, lastCol, programName, i, rank, BLOCKROWS, BLOCKCOLS);
        }
    }

    if(rank == 0) {
        MPI_Type_free(&blocktype);
        MPI_Type_free(&blocktype2);
    }

    MPI_Finalize();
 
    return 0;
}