
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define ROWS 6
#define COLS 7
#define PORT 8080

char board[ROWS][COLS];
int currentPlayer;
int gameStatus; // 0: in progress, 1: player wins, 2: bot wins, 3: draw
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Network function prototypes
void startNetworkCoop(int isServer, const char* ip);
void startServer();
void connectToServer(const char* ip);
void handleClient(int socket);
void handleServer(int clientSocket);

// Game function prototypes
void initializeBoard();
void printBoard();
int isMoveValid(int col);
void makeMove(int col, char playerSymbol);
int checkWin(char playerSymbol);
void* botThread(void* arg);
void saveGameState(const char* fileName);
void loadGameState(const char* fileName);

int main() {
    initializeBoard();
    printBoard();

    // Assuming network mode is chosen, replace with actual condition
    int networkMode = 1; // 0 for local, 1 for server, 2 for client
    if (networkMode == 1) {
        startNetworkCoop(1, NULL); // Start as server
    } else if (networkMode == 2) {
        startNetworkCoop(0, "127.0.0.1"); // Start as client, replace IP as needed
    } else {
        // Original game loop here for local play
         while (1) {
        // player's turn
        int playerMove;
        printf("Enter your move (column 1-7): ");
        scanf("%d", &playerMove);

        if (isMoveValid(playerMove - 1)) {
            makeMove(playerMove - 1, 'X');
            printBoard();
            usleep(500000); // 500 ms delay for animation

            if (checkWin('X')) {
                printf("Congratulations! You win!\n");
                saveGameState("game_state.txt");
                break;
            }
        } else {
            printf("Invalid move. Try again.\n");
            continue;
        }

        int draw = 1;
        for (int i = 0; i < COLS; ++i) {
            if (board[0][i] == ' ') {
                draw = 0;
                break;
            }
        }
        if (draw) {
            printf("It's a draw!\n");
            saveGameState("game_state.txt");
            break;
        }

        pthread_t botThreadID;
        pthread_create(&botThreadID, NULL, botThread, NULL);
        pthread_join(botThreadID, NULL);

        printBoard();
        usleep(500000); // 500 ms delay for animation

        if (checkWin('O')) {
            printf("The bot wins!\n");
            saveGameState("game_state.txt");
            break;
        }
    }
    }
    return 0;
}

// Network functions
void startNetworkCoop(int isServer, const char* ip) {
    if (isServer) {
        startServer();
    } else {
        connectToServer(ip);
    }
}

void startServer() {
    int serverFd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
      
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
      
    bind(serverFd, (struct sockaddr *)&address, sizeof(address));
    listen(serverFd, 3);
    
    newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    handleServer(newSocket);

    close(serverFd);
}

void connectToServer(const char* ip) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    inet_pton(AF_INET, ip, &serv_addr.sin_addr);
    
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    handleClient(sock);

    close(sock);
}

void handleServer(int clientSocket) {
    char buffer[1024] = {0};
    while (gameStatus == 0) {
        recv(clientSocket, buffer, 1024, 0);
        int clientMove = atoi(buffer);

        makeMove(clientMove, 'O');
        if (checkWin('O')) {
            gameStatus = 2; // bot wins
        }

        sprintf(buffer, "%d", gameStatus);
        send(clientSocket, buffer, strlen(buffer), 0);
    }
}

void handleClient(int sock) {
    char buffer[1024] = {0};
    while (gameStatus == 0) {
        sprintf(buffer, "%d", currentPlayer);
        send(sock, buffer, strlen(buffer), 0);

        recv(sock, buffer, 1024, 0);
        gameStatus = atoi(buffer);
    }
}

// Game functions (original implementations)
void saveGameState(const char* fileName) {
    FILE* file = fopen(fileName, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

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

void loadGameState(const char* fileName) {
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    fscanf(file, "%d", &currentPlayer);
    fscanf(file, "%d", &gameStatus);

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            fscanf(file, " %c", &board[i][j]);
        }
    }

    fclose(file);
}

void initializeBoard() {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            board[i][j] = ' ';
        }
    }
    currentPlayer = 1;
    gameStatus = 0;
}

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

int isMoveValid(int col) {
    return col >= 0 && col < COLS && board[0][col] == ' ';
}

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
