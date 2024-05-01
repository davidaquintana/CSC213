#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define ROWS 6
#define COLS 7

char board[ROWS][COLS];
int currentPlayer;
int gameStatus; // 0: in progress, 1: player wins, 2: bot wins, 3: draw

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//initilization of functions
void initializeBoard();
void printBoard();
int isMoveValid(int col);
void makeMove(int col, char playerSymbol);
int checkWin(char playerSymbol);
void* botThread(void* arg);
void saveGameState(const char* fileName);
void loadGameState(const char* fileName);


int main() {

    //initialize and print the starting board
    initializeBoard();
    printBoard();

    //whilerthere is a game to be played
    while (1) {
        // player's turn
        int playerMove;
        //UI
        printf("Enter your move (column 1-7): ");
        //UI Scan
        scanf("%d", &playerMove);


        //checks to see if move is valid    
        if (isMoveValid(playerMove - 1)) {
            //makes player'ssss move
            makeMove(playerMove - 1, 'X');
            //prints board after move
            printBoard();
            usleep(500000); // 500 ms delay for animation
            
            //checks to see if user won
            if (checkWin('X')) {
                printf("Congratulations! You win!\n");
                //saves game state
                saveGameState("game_state.txt");
                break;
            }
        //invlaid move contsraint
        } else {
            printf("Invalid move. Try again.\n");
            continue;
        }
        //checks to see if game ended in a draw
        int draw = 1;
        for (int i = 0; i < COLS; ++i) {
            if (board[0][i] == ' ') {
                draw = 0;
                break;
            }
        }
        //print draw
        if (draw) {
            printf("It's a draw!\n");
            saveGameState("game_state.txt");
            break;
        }
        
        //threads that allows the bot to take their turn
        pthread_t botThreadID;
        pthread_create(&botThreadID, NULL, botThread, NULL);
        pthread_join(botThreadID, NULL);

        printBoard();
        usleep(500000); // 500 ms delay for animation

        //if bot won or 0 user
        if (checkWin('O')) {
            printf("The bot wins!\n");
            //save gamestate
            saveGameState("game_state.txt");
            break;
        }
    }

    return 0;
}

//saving game state
void saveGameState(const char* fileName) {
    //opens fuile
    FILE* file = fopen(fileName, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    //prints game data into the file
    fprintf(file, "%d\n", currentPlayer);
    fprintf(file, "%d\n", gameStatus);

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            fprintf(file, "%c ", board[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

//loading game state
void loadGameState(const char* fileName) {
    //opens file
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    //scans in file data
    fscanf(file, "%d", &currentPlayer);
    fscanf(file, "%d", &gameStatus);

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            fscanf(file, " %c", &board[i][j]);
        }
    }

    fclose(file);
}

//initializes board
void initializeBoard() {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            board[i][j] = ' ';
        }
    }
    currentPlayer = 1;
    gameStatus = 0;
}

//prints board
void printBoard() {
    printf("\n  1   2   3   4   5   6   7\n");
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            printf("| %c ", board[i][j]);
        }
        printf("|\n");
    }
    printf("-----------------------------\n");
}

//checks move validity
int isMoveValid(int col) {
    return col >= 0 && col < COLS && board[0][col] == ' ';
}

//makes move
void makeMove(int col, char playerSymbol) {
    int row;
    for (row = 0; row < ROWS - 1; ++row) {
        if (board[row + 1][col] != ' ')
            break;
        printBoard();
        usleep(200000); // 200 ms delay for falling animation
        board[row][col] = ' ';
        board[row + 1][col] = playerSymbol;
    }
}

// checks if a player has won the game
int checkWin(char playerSymbol) {
    // check horizontally
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS - 3; ++j) {
            if (board[i][j] == playerSymbol &&
                board[i][j + 1] == playerSymbol &&
                board[i][j + 2] == playerSymbol &&
                board[i][j + 3] == playerSymbol) {
                return 1;
            }
        }
    }

    // check vertically
    for (int i = 0; i < ROWS - 3; ++i) {
        for (int j = 0; j < COLS; ++j) {
            if (board[i][j] == playerSymbol &&
                board[i + 1][j] == playerSymbol &&
                board[i + 2][j] == playerSymbol &&
                board[i + 3][j] == playerSymbol) {
                return 1;
            }
        }
    }

    // check diagonally (down-right)
    for (int i = 0; i < ROWS - 3; ++i) {
        for (int j = 0; j < COLS - 3; ++j) {
            if (board[i][j] == playerSymbol &&
                board[i + 1][j + 1] == playerSymbol &&
                board[i + 2][j + 2] == playerSymbol &&
                board[i + 3][j + 3] == playerSymbol) {
                return 1;
            }
        }
    }

    // check diagonally (up-right)
    for (int i = 3; i < ROWS; ++i) {
        for (int j = 0; j < COLS - 3; ++j) {
            if (board[i][j] == playerSymbol &&
                board[i - 1][j + 1] == playerSymbol &&
                board[i - 2][j + 2] == playerSymbol &&
                board[i - 3][j + 3] == playerSymbol) {
                return 1;
            }
        }
    }

    return 0;
}

//bot threads
void* botThread(void* arg) {
    pthread_mutex_lock(&mutex);

    int botMove;
    do {
        botMove = rand() % COLS;
    } while (!isMoveValid(botMove));

    makeMove(botMove, 'O');
    pthread_mutex_unlock(&mutex);

    return NULL;
}

