#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <mpi.h>
#include <ctime>
#include <vector>

size_t ROWS, COLS;

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


void updateBoard(char *board, int rows, int cols)
{
    std::vector<std::vector<int>> liveNeighbors(rows, std::vector<int>(cols, 0));

    //Count live neighbors
    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            if (((char)board[i*cols +j]) == 1)
            {
                for (int di = -1; di <= 1; ++di)
                {
                    for (int dj = -1; dj <= 1; ++dj)
                    {
                        //Periodic boundary conditions
                        liveNeighbors[(i + di + rows) % rows][(j + dj + cols) % cols]++;
                    }
                }
                liveNeighbors[i][j]--; //Correction so that a cell does not concider itself as a live neighbor
            }
        }
    }

    //Update board
    for (size_t i = 0; i < rows; ++i)
    {
        for (size_t j = 0; j < cols; ++j)
        {
            board[i*cols + j] = (char)((liveNeighbors[i][j] == 3) || (board[i*cols +j] && liveNeighbors[i][j] == 2));
        }
    }
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
    int numtasks, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // get partition values
    const int NPROWS = sqrt(numtasks);  // number of processors is squared number
    const int NPCOLS = sqrt(numtasks);
    const int BLOCKROWS = ROWS / NPROWS;  // 
    const int BLOCKCOLS = COLS / NPCOLS;

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
        initializeBoard(&board[0][0], ROWS, COLS);
    
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
    MPI_Type_vector(BLOCKROWS, BLOCKCOLS, COLS, MPI_CHAR, &blocktype2);
    MPI_Type_create_resized(blocktype2, 0, sizeof(char), &blocktype);
    MPI_Type_commit(&blocktype);


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
    
    // //Do iteration
    // writeBoardToFile(&locBoard[0][0], firstRow, lastRow, firstCol, lastCol, programName, 0, rank, BLOCKROWS, BLOCKCOLS);
    // for (int i = 1; i <= iterations; ++i)
    // {
    //     updateBoard(&locBoard[0][0], BLOCKROWS, BLOCKCOLS);
    //     if (i % iteration_gap == 0)
    //     {
    //         writeBoardToFile(&locBoard[0][0], firstRow, lastRow, firstCol, lastCol, programName, i, rank, BLOCKROWS, BLOCKCOLS);
    //     }
    // }

    MPI_Finalize();
 
    return 0;
}