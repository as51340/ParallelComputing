#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <mpi.h>
#include <ctime>
#include <vector>
#include "test.h"


int ROWS, COLS;
int BLOCKROWS, BLOCKCOLS;
int NPROWS, NPCOLS;
int NUMTASKS;


// Name of the program - all threads will use same program
std::string programName;
    
int upTag = 0, rightTag = 1, downTag = 2, leftTag = 3; // from sending perspective
int upLeftTag = 4, upRightTag = 5, downRightTag = 6, downLeftTag = 7;


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

void updateBoard(char *board, int rank, int firstRow, int lastRow, int firstCol, int lastCol, int iteration, MPI_Datatype col_type)
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


    // DIAGONAL COMMUNICATION
    MPI_Request diagReqs[4];
    MPI_Status diagStats[4];
    // MPI_Request diagReqsRecv[4];
    // MPI_Status diagStatsRecv[4];


    
    char **comm = new char*[4];
    char diagBuffer[4];

    // FIRST SEND EVERYTHING
    // SEND UP
    size_t dest = 0;
    dest = (NPCOLS * (NPROWS - 1) + rank) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Isend(&board[0], BLOCKCOLS, MPI_CHAR, dest, upTag, MPI_COMM_WORLD, &reqs[upTag]); // send up

    // SEND DOWN
    dest = (rank + NPCOLS) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Isend(&board[BLOCKCOLS*(BLOCKROWS-1)], BLOCKCOLS, MPI_CHAR, dest, downTag, MPI_COMM_WORLD, &reqs[downTag]);  // send down

    // SEND LEFT
    dest = (rank + NPCOLS - 1) % NPCOLS; // COLUMN DEST UPDATE
    MPI_Isend(&(board[0]), 1, col_type, dest, leftTag, MPI_COMM_WORLD, &reqs[leftTag]); // send left

    // SEND RIGHT
    dest = (rank + NPCOLS + 1) % NPCOLS; // COLUMN DEST UPDATE
    MPI_Isend(&(board[BLOCKCOLS-1]), 1, col_type, dest, rightTag, MPI_COMM_WORLD, &reqs[rightTag]); // send right


    // TRY TO SEND DIAGONALS
    // SEND UPPER LEFT, i - 1, j - 1
    dest = (rank + NPCOLS - 1) % NPCOLS; // COLUMN DEST UPDATE
    dest = (NPCOLS * (NPROWS - 1) + dest) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Isend(&board[0], 1, MPI_CHAR, dest, upLeftTag, MPI_COMM_WORLD, &diagReqs[upLeftTag]); // SEND UPPER LEFT ELEMENT TO UPPER LEFT PROCESSOR AND SAVE IT AS upLeftTag

    // SEND UPPER RIGHT: i-1, j+1
    dest = (rank + NPCOLS + 1) % NPCOLS; // COLUMN DEST UPDATE
    dest = (NPCOLS * (NPROWS - 1) + dest) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Isend(&board[BLOCKCOLS-1], 1, MPI_CHAR, dest, upRightTag, MPI_COMM_WORLD, &diagReqs[upRightTag]); // SEND UPPER RIGHT ELEMENT TO UPPER RIGHT PROCESSOR AND SAVE IT AS upRightTag

    // SEND DOWN RIGHT: i+1, j+1
    dest = (rank + NPCOLS + 1) % NPCOLS; // COLUMN DEST UPDATE
    dest = (dest + NPCOLS) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Isend(&board[BLOCKCOLS*BLOCKROWS-1], 1, MPI_CHAR, dest, downRightTag, MPI_COMM_WORLD, &diagReqs[downRightTag]); // SEND DOWN RIGHT ELEMENT TO DOWN RIGHT PROCESSOR AND SAVE IT AS downRightTag

    // SEND DOWN LEFT: i+1, j-1
    dest = (rank + NPCOLS - 1) % NPCOLS; // COLUMN DEST UPDATE
    dest = (dest + NPCOLS) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Isend(&board[(BLOCKROWS-1)*BLOCKCOLS], 1, MPI_CHAR, dest, downLeftTag, MPI_COMM_WORLD, &diagReqs[downLeftTag]); // SEND DOWN LEFT ELEMENT TO DOWN LEFT PROCESSOR AND SAVE IT AS downLeftTag


    // AFTER THAT SEND REQUESTS FOR RECEIVING
    // SEND RECEIVE FROM UP
    size_t src = 0;
    src = (NPCOLS * (NPROWS - 1) + rank) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    comm[upTag] = new char[BLOCKCOLS];
    MPI_Irecv(&comm[upTag][0], BLOCKCOLS, MPI_CHAR, src, downTag, MPI_COMM_WORLD, &recvReqs[upTag]); // receive from up

    // SEND RECEIVE FROM DOWN
    src = (rank + NPCOLS) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    comm[downTag] = new char[BLOCKCOLS];
    MPI_Irecv(&comm[downTag][0], BLOCKCOLS, MPI_CHAR, src, upTag, MPI_COMM_WORLD, &recvReqs[downTag]); // receive from down

    // SEND RECEIVE FROM LEFT
    src = (rank + NPCOLS - 1) % NPCOLS; // COLUMN DEST UPDATE
    comm[leftTag] = new char[BLOCKROWS];
    MPI_Irecv(&(comm[leftTag][0]), BLOCKROWS, MPI_CHAR, src, rightTag, MPI_COMM_WORLD, &recvReqs[leftTag]);


    // SEND RECEIVE FROM RIGHT
    src = (rank + NPCOLS + 1) % NPCOLS; // COLUMN DEST UPDATE
    comm[rightTag] = new char[BLOCKROWS]; 
    MPI_Irecv(&(comm[rightTag][0]), BLOCKROWS, MPI_CHAR, src, leftTag, MPI_COMM_WORLD, &recvReqs[rightTag]);


    // DIAGONALS RECEIVING
    // RECEIVE FROM UP LEFT
    src = (rank + NPCOLS - 1) % NPCOLS; // COLUMN DEST UPDATE
    src = (NPCOLS * (NPROWS - 1) + src) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Irecv(&diagBuffer[0], 1, MPI_CHAR, src, upLeftTag, MPI_COMM_WORLD, &diagReqsRecv[0]);

    // RECEIVE FROM UP RIGHT
    src = (rank + NPCOLS + 1) % NPCOLS; // COLUMN DEST UPDATE
    src = (NPCOLS * (NPROWS - 1) + src) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Irecv(&diagBuffer[1], 1, MPI_CHAR, src, upRightTag, MPI_COMM_WORLD, &diagReqsRecv[1]);


    // RECEIVE FROM DOWN RIGHT
    src = (rank + NPCOLS + 1) % NPCOLS; // COLUMN DEST UPDATE
    src = (src + NPCOLS) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Irecv(&diagBuffer[2], 1, MPI_CHAR, src, downRightTag, MPI_COMM_WORLD, &diagReqsRecv[2]);

    // RECEIVE FROM DOWN LEFT
    src = (rank + NPCOLS -1) % NPCOLS; // COLUMN DEST UPDATE
    src = (src + NPCOLS) % (NPCOLS*NPROWS); // ROW DEST UPDATE
    MPI_Irecv(&diagBuffer[3], 1, MPI_CHAR, src, downLeftTag, MPI_COMM_WORLD, &diagReqsRecv[3]);


    // NOW PERFORM LIVE NEIGHBOURS UPDATE FROM VALUES YOU KNOW
    std::vector<std::vector<int>> liveNeighbors(BLOCKROWS, std::vector<int>(BLOCKCOLS, 0));
    //Count live neighbors
    for (size_t i = 0; i < BLOCKROWS; ++i)
    {
        for (size_t j = 0; j < BLOCKCOLS; ++j)
        {
            if (((char)board[i*BLOCKCOLS +j]) == 1)
            {   // HORIZONTAL AND VERTICAL UPDATES
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
                // DIAGONAL UPDATES
                if(i > 0 && j > 0) {
                    liveNeighbors[i-1][j-1]++;
                }
                if(i > 0 && j < BLOCKCOLS - 1) {
                    liveNeighbors[i-1][j+1]++;
                }
                if(i < BLOCKROWS - 1 && j < BLOCKCOLS - 1) {
                    liveNeighbors[i+1][j+1]++;
                }
                if(i < BLOCKROWS - 1 && j > 0) {
                    liveNeighbors[i+1][j-1]++;
                }
            }
        }
    }


    // NOW PERFORM UPDATE OF THE PART OF THE BOARD YOU KNOW HOW TO SOLVE
    //Update board
    for (size_t i = 1; i < BLOCKROWS-1; ++i) {
        for (size_t j = 1; j < BLOCKCOLS-1; ++j) {
            board[i*BLOCKCOLS + j] = (char)((liveNeighbors[i][j] == 3) || ((char)board[i*BLOCKCOLS +j]) == 1 && liveNeighbors[i][j] == 2);
        }
    }

    // NOW WAIT FOR NEIGHBOURS VECTOR SO YOU CAN PERFORM FINAL UPDATE
    // WAIT FOR VERTICAL COMMUNICATION
    MPI_Wait(&recvReqs[upTag], &recvStats[upTag]);
    MPI_Wait(&recvReqs[downTag], &recvStats[downTag]);

    // PRINT WHAT I RECEIVED FROM UP PROCESSOR
    std::cout << "Proc: " << rank << " received from up processor, iteration " << iteration << std::endl;
    std::cout << "----------" << std::endl;
    for(int i = 0; i < BLOCKCOLS; i++) {
        printf("%3d ", (char)comm[upTag][i]);
    }
    std::cout << std::endl << "----------" << std::endl;

    std::cout << "Proc: " << rank << " received from down processor, iteration " << iteration << std::endl;
    std::cout << "----------" << std::endl;
    for(int i = 0; i < BLOCKCOLS; i++) {
        printf("%3d ", (char)comm[downTag][i]);
    }
    std::cout << std::endl << "----------" << std::endl;

    // UPDATE FIRST AND LAST ROWS LIVE NEIGHBOURS
    for(int j = 0; j < BLOCKCOLS; j++) {
        if(j > 0) {
            liveNeighbors[0][j-1] += (int)comm[upTag][j];
            liveNeighbors[BLOCKROWS-1][j-1] += (int)comm[downTag][j];
        }
        liveNeighbors[0][j] += (int)comm[upTag][j];
        liveNeighbors[BLOCKROWS-1][j] += (int)comm[downTag][j];
        if(j < BLOCKCOLS - 1) {
            liveNeighbors[0][j+1] += (int)comm[upTag][j];
            liveNeighbors[BLOCKROWS-1][j+1] += (int)comm[downTag][j];
        }
    }

    // WAIT FOR HORIZONTAL COMMUNICATION
    MPI_Wait(&recvReqs[rightTag], &recvStats[rightTag]);
    MPI_Wait(&recvReqs[leftTag], &recvStats[leftTag]);

    std::cout << "Proc: " << rank << " received from left processor, iteration " << iteration << std::endl;
    std::cout << "----------" << std::endl;
    for(int i = 0; i < BLOCKROWS; i++) {
        printf("%3d ", (char)comm[leftTag][i]);
    }
    std::cout << std::endl << "----------" << std::endl;

    
    std::cout << "Proc: " << rank << " received from right processor, iteration " << iteration << std::endl;
    std::cout << "----------" << std::endl;
    for(int i = 0; i < BLOCKROWS; i++) {
        printf("%3d ", (char)comm[rightTag][i]);
    }
    std::cout << std::endl << "----------" << std::endl;

    // UPDATE FIRST AND LAST COLUMN OF LIVE NEIGHBOURS
    for(int i = 0; i < BLOCKROWS; i++) {
        if(i > 0 ){
            liveNeighbors[i-1][0] += (int)comm[leftTag][i];    
            liveNeighbors[i-1][BLOCKCOLS-1] += (int)comm[rightTag][i];
        }
        liveNeighbors[i][0] += (int)comm[leftTag][i];
        liveNeighbors[i][BLOCKCOLS-1] += (int)comm[rightTag][i];
        if(i < BLOCKROWS - 1) {
            liveNeighbors[i+1][0] += (int)comm[leftTag][i];
            liveNeighbors[i+1][BLOCKCOLS-1] += (int)comm[rightTag][i];
        }
    }

    // UPDATE REMAINING PART OF BOARD
    // UPDATE FIRST AND LAST ROW OF BOARD
    for(int j = 0; j < BLOCKCOLS; j++) {
        // FIRST ROW
        board[j] = (char)((liveNeighbors[0][j] == 3) || ((char)board[j]) == 1 && liveNeighbors[0][j] == 2);
        // LAST ROW
        board[(BLOCKROWS-1)*BLOCKCOLS + j] = (char)((liveNeighbors[BLOCKROWS-1][j] == 3) || ((char)board[(BLOCKROWS-1)*BLOCKCOLS + j]) == 1 && liveNeighbors[BLOCKROWS-1][j] == 2);
    }

    // UPDATE FIRST AND LAST COLUMN OF BOARD
    for(int i = 0; i < BLOCKROWS; i++) {
        // UPDATE FIRST COLUMN
        board[i*BLOCKCOLS] = (char)((liveNeighbors[i][0] == 3) || ((char)board[i*BLOCKCOLS]) == 1 && liveNeighbors[i][0] == 2);
        // UPDATE LAST COLUMN
        board[BLOCKCOLS*(i+1) - 1] = (char)((liveNeighbors[i][BLOCKCOLS-1] == 3) || ((char)board[BLOCKCOLS*(i+1) - 1]) == 1 && liveNeighbors[i][BLOCKCOLS-1] == 2);
    }

    // Wait for all send request to finish and free memory in which data was received
    for(int i = 0; i < 4; i++) {
        MPI_Wait(&reqs[i], &stats[i]);
        delete comm[i];
    }
    
    // final clear
    delete comm;
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

    MPI_Status Stat;
    int firstRow, lastRow, firstCol, lastCol;

    size_t rowTag = 0, colTag = 1;
    int programNameTag = 2;

    if(rank == 0) {
        std::cout << "Number of processors: " << NUMTASKS << std::endl;
        std::cout << "NPROWS: " << NPROWS << std::endl;
        std::cout << "NPCOLS: " << NPCOLS << std::endl;
        std::cout << "BlOCKROWS: " << BLOCKROWS << std::endl;
        std::cout << "BLOCKCOLS: " << BLOCKCOLS << std::endl;

        // init_board(&board[0][0], ROWS, COLS);
        // initializeBoard(&board[0][0], ROWS, COLS);
        test2_init_board(&board[0][0]);
    
        std::cout << "Global matrix" << std::endl;
        print_board(&board[0][0], ROWS, COLS);
        std::cout << std::endl;

        // Only one program name
        programName = setUpProgram(ROWS, COLS, iteration_gap, iterations, NUMTASKS);

        // Send offsets from processor 0 to all other processors
        for(size_t i = 0; i < NPROWS; i++) {
            for(size_t j = 0; j < NPCOLS; j++) {
                int locFirstRow = BLOCKROWS * i;
                int locFirstCol = BLOCKCOLS * j;
                int procNum = i*NPCOLS + j;
                // SEND DATA ABOUT ROW AND COL
                MPI_Send(&locFirstRow, 1, MPI_INT, procNum, rowTag, MPI_COMM_WORLD);
                MPI_Send(&locFirstCol, 1, MPI_INT, procNum, colTag, MPI_COMM_WORLD);
                // SEND PROGRAM NAME TO ALL OTHER PROCESSORS
                MPI_Send(programName.c_str(), programName.size(), MPI_CHAR, procNum, programNameTag, MPI_COMM_WORLD);
            }
        }
    }

    // Receive first row variable and first col variable
    MPI_Recv(&firstRow, 1, MPI_INT, 0, rowTag, MPI_COMM_WORLD, &Stat);
    MPI_Recv(&firstCol, 1, MPI_INT, 0, colTag, MPI_COMM_WORLD, &Stat);
    // RECEIVE PROGRAM NAME FROM ROOT PROCESSOR 
    MPI_Status programNameStatus;
    MPI_Probe(0, programNameTag, MPI_COMM_WORLD, &programNameStatus);
    int programNameLen;
    MPI_Get_count(&programNameStatus, MPI_CHAR, &programNameLen);

    char programNameBuffer[programNameLen];
    MPI_Recv(&programNameBuffer[0], programNameLen, MPI_CHAR, 0, programNameTag, MPI_COMM_WORLD, &Stat);

    programName = std::string(programNameBuffer, programNameLen);

    std::cout << "Rank: " << rank << " Program name: " << programName << std::endl;



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

    // SYNCHRONIZE ALL CORES IN THIS MOMENT
    MPI_Barrier(MPI_COMM_WORLD);

    // //Do iteration
    writeBoardToFile(&locBoard[0][0], firstRow, lastRow, firstCol, lastCol, programName, 0, rank, BLOCKROWS, BLOCKCOLS);
    for (int i = 1; i <= iterations; ++i)
    {
        updateBoard(&locBoard[0][0], rank, firstRow, lastRow, firstCol, lastCol, i, col_type);
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