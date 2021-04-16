#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <mpi.h>

#define CHESSBOARD_SIZE_MAX 13 * 13
#define MOVES_SIZE_MAX 22
#define STATE_SIZE_MAX 24

int bestDepth;
int *bestMoves;

typedef struct {
    char board[CHESSBOARD_SIZE_MAX];
    int boardSize;

    int depth;
    int maxDepth;
    int moves[MOVES_SIZE_MAX];

    int knightIndex;
    int bishopIndex;
    char figureMove;

    int boardFigures;
    int takeFiguresCount;

} ChessSearchState;

typedef struct {
    int depth;
    int moves[MOVES_SIZE_MAX];
} ChessSearchResult;

#pragma region Queue
struct Queue {
    int front, rear, size;
    unsigned capacity;
    ChessSearchState *array;
};

struct Queue *createQueue(unsigned capacity) {
    struct Queue *queue = (struct Queue *) malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (ChessSearchState *) malloc(queue->capacity * sizeof(ChessSearchState));
    return queue;
}

int isFull(struct Queue *queue) {
    return (queue->size == queue->capacity);
}

int isEmpty(struct Queue *queue) {
    return (queue->size == 0);
}

void enqueue(struct Queue *queue, ChessSearchState item) {
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}

ChessSearchState dequeue(struct Queue *queue) {
    if (isEmpty(queue)) {
        ChessSearchState defaultChess;
        defaultChess.depth = -1;
        return defaultChess;
    }

    ChessSearchState item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

ChessSearchState front(struct Queue *queue) {
    if (isEmpty(queue)) {
        ChessSearchState defaultChess;
        defaultChess.depth = -1;
        return defaultChess;
    }
    return queue->array[queue->front];
}

ChessSearchState rear(struct Queue *queue) {
    if (isEmpty(queue)) {
        ChessSearchState defaultChess;
        defaultChess.depth = -1;
        return defaultChess;
    }
    return queue->array[queue->rear];
}

#pragma endregion Queue

typedef struct {
    int rowIndex;
    int colIndex;
} Position;

#pragma region Helpers

void copyIntArray(const int *source, int *target, int size) {
    for (int i = 0; i < size; i++) {
        target[i] = source[i];
    }
}

void copyArray(const char *source, char *target, int size) {
    for (int i = 0; i < size; i++) {
        target[i] = source[i];
    }
}

void swap(int *xp, int *yp) {
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void swapState(ChessSearchState *xp, ChessSearchState *yp) {
    ChessSearchState temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void bubbleSort(int *scores, int *successors, int n) {
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (scores[j] <= scores[j + 1]) {
                swap(&successors[j], &successors[j + 1]);
                swap(&scores[j], &scores[j + 1]);
            }
        }
    }
}

#pragma endregion Helpers

int mapIndex(int i, int j, int chessboardSize) {
    return i * chessboardSize + j;
}

Position mapIndexInverse(int index, int chessboardSize) {
    Position position;
    position.rowIndex = index / chessboardSize;
    position.colIndex = index % chessboardSize;

    return position;
}

bool isPositionValid(int rowIndex, int colIndex, int chessboardSize) {
    return rowIndex < chessboardSize
           && rowIndex >= 0
           && colIndex < chessboardSize
           && colIndex >= 0;
}

int calculateScore(char *chessboard, int chessboardSize, int successor, char movingFigure) {
    if (chessboard[successor] == 'P')
        return 2;

    if (movingFigure == 'J')
        return 0;

    bool isDiagAFree = true;
    bool isDiagBFree = true;
    bool isDiagCFree = true;
    bool isDiagDFree = true;

    for (int i = 0; i < chessboardSize; i++) {
        Position position = mapIndexInverse(successor, chessboardSize);
        if (isPositionValid(position.rowIndex + i, position.colIndex + i, chessboardSize) && isDiagAFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagAFree = false;
            if (chessboard[index] == 'P') return 1;
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex - i, chessboardSize) && isDiagBFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagBFree = false;
            if (chessboard[index] == 'P') return 1;
        }

        if (isPositionValid(position.rowIndex + i, position.colIndex - i, chessboardSize) && isDiagCFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagCFree = false;
            if (chessboard[index] == 'P') return 1;
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex + i, chessboardSize) && isDiagDFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagDFree = false;
            if (chessboard[index] == 'P') return 1;
        }
    }

    return 0;
}

void val(char *chessboard, int chessboardSize, int *successors, int successorsSize, char nextMove) {
    int *scores = (int *) malloc(sizeof(int) * successorsSize);
    for (int i = 0; i < successorsSize; i++) {
        scores[i] = calculateScore(chessboard, chessboardSize, successors[i], nextMove);
    }

    bubbleSort(scores, successors, successorsSize);
    free(scores);
}

int move(char *chessboard, int startMoveIndex, int endMoveIndex) {
    int taken = 0;
    if (chessboard[endMoveIndex] == 'P') taken++;

    char figureStart = chessboard[startMoveIndex];

    chessboard[startMoveIndex] = '-';
    chessboard[endMoveIndex] = figureStart;

    return taken;
}

#pragma region PrintHelpers

void printChessboard(char *chessboard, int chessboardSize) {
    printf("\n");
    for (int i = 0; i < chessboardSize * chessboardSize; i++) {
        printf("%c", chessboard[i]);
        if ((i + 1) % chessboardSize == 0)
            printf("\n");
    }
    printf("\n");
}

void printFigureCapturedMove(int moveOrder, char figure, Position startPosition, Position endPosition) {
    printf("%d. %c: [%d, %d] -> [%d, %d]*\n", moveOrder + 1, figure, startPosition.rowIndex, startPosition.colIndex,
           endPosition.rowIndex, endPosition.colIndex);
}

void printFigureMove(int moveOrder, char figure, Position startPosition, Position endPosition) {
    printf("%d. %c: [%d, %d] -> [%d, %d]\n", moveOrder + 1, figure, startPosition.rowIndex, startPosition.colIndex,
           endPosition.rowIndex, endPosition.colIndex);
}

void printMoves(int *moves, int movesCount, int chessboardSize, int knightIndex, int bishopIndex) {
    char figure;
    Position knightPos = mapIndexInverse(knightIndex, chessboardSize);
    Position bishopPos = mapIndexInverse(bishopIndex, chessboardSize);

    for (int i = 0; i < movesCount; i++) {
        figure = i % 2 == 0 ? 'S' : 'J';
        Position nextMovePos;
        if (moves[i] > 0) {
            nextMovePos = mapIndexInverse(moves[i], chessboardSize);
        } else {
            nextMovePos = mapIndexInverse(moves[i] * -1, chessboardSize);
        }

        if (i == 0 && moves[i] > 0) {
            printFigureCapturedMove(i, figure, bishopPos, nextMovePos);
        } else if (i == 0 && moves[i] <= 0) {
            printFigureMove(i, figure, bishopPos, nextMovePos);
        }

        if (i == 1 && moves[i] > 0) {
            printFigureCapturedMove(i, figure, knightPos, nextMovePos);
        } else if (i == 1 && moves[i] <= 0) {
            printFigureMove(i, figure, knightPos, nextMovePos);
        }

        if (i > 1) {
            Position startPosition;
            if (moves[i - 2] > 0) {
                startPosition = mapIndexInverse(moves[i - 2], chessboardSize);
            } else {
                startPosition = mapIndexInverse(moves[i - 2] * -1, chessboardSize);
            }

            if (moves[i] > 0) {
                printFigureCapturedMove(i, figure, startPosition, nextMovePos);
            } else if (moves[i] <= 0) {
                printFigureMove(i, figure, startPosition, nextMovePos);
            }
        }
    }
}

#pragma endregion PrintHelpers

bool canMove(char figurine) {
    return figurine != 'J' && figurine != 'S';
}

// get next moves for a bishop
int nextBishop(char *chessboard, int chessboardSize, int *successors, int startIndex) {
    Position position = mapIndexInverse(startIndex, chessboardSize);
    int nextMoveCount = 0;

    bool isDiagAFree = true;
    bool isDiagBFree = true;
    bool isDiagCFree = true;
    bool isDiagDFree = true;

    for (int i = 0; i < chessboardSize; i++) {
        if (isPositionValid(position.rowIndex + i, position.colIndex + i, chessboardSize) && isDiagAFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagAFree = false;
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex - i, chessboardSize) && isDiagBFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagBFree = false;
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }

        if (isPositionValid(position.rowIndex + i, position.colIndex - i, chessboardSize) && isDiagCFree) {
            int index = mapIndex(position.rowIndex + i, position.colIndex - i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagCFree = false;
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }

        if (isPositionValid(position.rowIndex - i, position.colIndex + i, chessboardSize) && isDiagDFree) {
            int index = mapIndex(position.rowIndex - i, position.colIndex + i, chessboardSize);
            if (chessboard[index] == 'P' || chessboard[index] == 'J') isDiagDFree = false;
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }
    }

    return nextMoveCount;
}

// get next moves for a knight
int nextKnight(char *chessboard, int chessboardSize, int *successors, int startIndex) {
    int moves[4] = {-1, -2, 1, 2};
    int movesSize = sizeof(moves) / sizeof(moves[0]);
    Position position = mapIndexInverse(startIndex, chessboardSize);

    int nextMoveCount = 0;
    for (int i = 0; i < movesSize; i++) {
        for (int j = 0; j < movesSize; j++) {
            if (moves[i] + moves[j] == 0 || moves[i] == moves[j])
                continue;

            if (!isPositionValid(position.rowIndex + moves[i], position.colIndex + moves[j], chessboardSize))
                continue;

            int index = mapIndex(position.rowIndex + moves[i], position.colIndex + moves[j], chessboardSize);
            if (canMove(chessboard[index])) {
                successors[nextMoveCount] = index;
                nextMoveCount++;
            }
        }
    }

    return nextMoveCount;
}

// get next moves for a knight or a bishop depends on position
int next(char *chessboard, int chessboardSize, int *successors, int startIndex) {
    //int index = mapIndex(position.rowIndex, position.colIndex, boardSize);
    if (chessboard[startIndex] == 'J') {
        return nextKnight(chessboard, chessboardSize, successors, startIndex);
    } else if (chessboard[startIndex] == 'S') {
        return nextBishop(chessboard, chessboardSize, successors, startIndex);
    } else {
        printf("Invalid position '%c' for J/S.\n", chessboard[startIndex]);
        return -1;
    }
}

void createStateFromPredecessor(ChessSearchState *state, ChessSearchState *nextState, int nextMoveIndex,
                                int capturedFigure) {
    char nextMove;
    int nextBishopIndex = state->bishopIndex;
    int nextKnightIndex = state->knightIndex;
    if (state->figureMove == 'S') {
        nextBishopIndex = nextMoveIndex;
        nextMove = 'J';
    } else {
        nextKnightIndex = nextMoveIndex;
        nextMove = 'S';
    }

    nextState->boardSize = state->boardSize;
    nextState->bishopIndex = nextBishopIndex;
    nextState->knightIndex = nextKnightIndex;
    nextState->figureMove = nextMove;
    nextState->depth = state->depth + 1;
    nextState->maxDepth = state->maxDepth;
    nextState->boardFigures = state->boardFigures;
    nextState->takeFiguresCount = state->takeFiguresCount + capturedFigure;
}

void recordMove(int *moves, int move, int depth, int capturedFigure) {
    if (capturedFigure == 0) {
        moves[depth] = move * -1;
    } else {
        moves[depth] = move;
    }
}

int getStartIndex(ChessSearchState previousState) {
    if (previousState.figureMove == 'S') {
        return previousState.bishopIndex;
    } else {
        return previousState.knightIndex;
    }
}

void dfsChessboard(ChessSearchState state) {
    // lower bound
    if (state.depth == state.boardFigures && state.boardFigures == state.takeFiguresCount) {
#pragma omp critical
        {
            if (state.depth == state.boardFigures &&
                state.boardFigures == state.takeFiguresCount) {
                bestDepth = state.depth;
                copyIntArray(state.moves, bestMoves, state.maxDepth);
            }
        }
        return;
    }

    // upper bound (init best depth is max depth)
    if (state.depth >= bestDepth) {
        return;
    }

    // update best depth
    if (state.boardFigures == state.takeFiguresCount && state.depth < bestDepth) {
#pragma omp critical
        {
            if (state.boardFigures == state.takeFiguresCount && state.depth < bestDepth) {
                bestDepth = state.depth;
                copyIntArray(state.moves, bestMoves, state.maxDepth);
            }
        }
        return;
    }

    // Branch and bound
    if (state.depth + (state.boardFigures - state.takeFiguresCount) >= bestDepth) {
        return;
    }

    int startMoveIndex = getStartIndex(state);
    int *successors;
    if (state.figureMove == 'S') {
        successors = (int *) malloc(sizeof(int) * (2 * state.boardSize - 2));
    } else {
        successors = (int *) malloc(sizeof(int) * 8);
    }

    // get all possible moves
    int successorSize = next(state.board, state.boardSize, successors, startMoveIndex);
    // Is figureMove correct param?
    val(state.board, state.boardSize, successors, successorSize, state.figureMove);

    for (int i = 0; i < successorSize; i++) {
        ChessSearchState newState;
        copyArray(state.board, newState.board, state.boardSize * state.boardSize);
        copyIntArray(state.moves, newState.moves, state.maxDepth);

        int startPosition = state.figureMove == 'S' ? state.bishopIndex : state.knightIndex;
        int capturedFigure = move(newState.board, startPosition, successors[i]);
        recordMove(newState.moves, successors[i], state.depth, capturedFigure);

        createStateFromPredecessor(&state, &newState, successors[i], capturedFigure);

        dfsChessboard(newState);
    }

    free(successors);
}

int getNextStates(ChessSearchState previousState, ChessSearchState *nextStates) {
    int startMoveIndex = getStartIndex(previousState);

    int *successors = (int *) malloc(sizeof(int) * (2 * previousState.boardSize - 2));
    int successorsSize = next(previousState.board, previousState.boardSize, successors, startMoveIndex);

    val(previousState.board, previousState.boardSize, successors, successorsSize,
        previousState.figureMove == 'S' ? 'J' : 'S');

    for (int i = 0; i < successorsSize; i++) {
        copyArray(previousState.board, nextStates[i].board, previousState.boardSize * previousState.boardSize);
        copyIntArray(previousState.moves, nextStates[i].moves, previousState.maxDepth);

        int capturedFigure = move(nextStates[i].board, startMoveIndex, successors[i]);
        recordMove(nextStates[i].moves, successors[i], previousState.depth, capturedFigure);

        createStateFromPredecessor(&previousState, &nextStates[i], successors[i], capturedFigure);
    }

    free(successors);

    return successorsSize;
}

void bfsChessboard(struct Queue *queue, ChessSearchState *state) {
    enqueue(queue, *state);

    while (!isFull(queue)) {
        ChessSearchState nextStates[STATE_SIZE_MAX];
        int nextStateSize = getNextStates(dequeue(queue), nextStates);

        for (int i = 0; i < nextStateSize; i++) {
            enqueue(queue, nextStates[i]);
        }
    }

    for (int i = 0; i < queue->capacity - 1; i++) {
        for (int j = 0; j < queue->capacity - i - 1; j++) {
            if (queue->array[i].takeFiguresCount <= queue->array[j + 1].takeFiguresCount) {
                swapState(&queue->array[j], &queue->array[j + 1]);
            }
        }
    }
}

void parseInstance(ChessSearchState *state, int argc, char *argv[]) {
    int chessboardStartArg = 3;
    char *p_chessboardSize;
    p_chessboardSize = *(argv + 1);

    char *p_maxDepth;
    p_maxDepth = *(argv + 2);

    state->depth = 0;
    state->figureMove = 'S';
    state->boardFigures = 0;
    state->takeFiguresCount = 0;
    state->boardSize = atoi(p_chessboardSize);
    state->maxDepth = atoi(p_maxDepth);

// Parse instance from CMD
    for (int i = chessboardStartArg; i < argc; i++) {
        int size = strlen(argv[i]);
        for (int j = 0; j < size; j++) {
            int index = mapIndex(i - chessboardStartArg, j, state->boardSize);
            state->board[index] = argv[i][j];
            if (state->board[index] == 'J') {
                state->knightIndex = mapIndex(i - chessboardStartArg, j, state->boardSize);
            }
            if (state->board[index] == 'S') {
                state->bishopIndex = mapIndex(i - chessboardStartArg, j, state->boardSize);
            }
            if (state->board[index] == 'P')
                state->boardFigures++;
        }
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, 0);

    int tagWork = 0;
    int tagFinished = 1;
    ChessSearchState message;
    struct Queue *queue;
    MPI_Status status;

    clock_t start = clock();

    MPI_Init(&argc, &argv);

    int processRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);

    int processCount;
    MPI_Comm_size(MPI_COMM_WORLD, &processCount);

    // Master
    if (processRank == 0) {
        parseInstance(&message, argc, argv);

        // Initialize bestResult
        ChessSearchResult bestResult;
        bestResult.depth = message.maxDepth;
        ChessSearchResult resultBuffer;

        int workingSlaves = processCount - 1;
        int statesCount = workingSlaves * 10;
        queue = createQueue(statesCount);
        bfsChessboard(queue, &message);

        // send the job to all slaves
        for (int i = 1; i < processCount; i++) {
            ChessSearchState dequeuedState = dequeue(queue);
            MPI_Send(&dequeuedState, sizeof(ChessSearchState), MPI_PACKED, i, tagWork, MPI_COMM_WORLD);
        }

        while (workingSlaves > 0) {
            // until any slave working, wait for the job result
            MPI_Recv(&resultBuffer, sizeof(ChessSearchResult), MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG,
                     MPI_COMM_WORLD, &status);

            // refresh the best result
            if (bestResult.depth > resultBuffer.depth) {
                bestResult.depth = resultBuffer.depth;
                copyIntArray(resultBuffer.moves, bestResult.moves, MOVES_SIZE_MAX);
            }

            // if there is more job, then send a job to the free slave
            int freeSlave = status.MPI_SOURCE;
            if (!isEmpty(queue)) {
                ChessSearchState dequeuedState = dequeue(queue);
                MPI_Send(&dequeuedState, sizeof(ChessSearchState), MPI_PACKED, freeSlave, tagWork, MPI_COMM_WORLD);
            } else {
                // if there is no job, then send the end request to the free slave
                ChessSearchState endState;
                MPI_Send(&endState, sizeof(ChessSearchState), MPI_PACKED, freeSlave, tagFinished, MPI_COMM_WORLD);
                workingSlaves--;
            }
        }

        free(queue->array);
        free(queue);

        printMoves(bestResult.moves, bestResult.depth, message.boardSize, message.knightIndex, message.bishopIndex);
    } else {
        while (true) {
            // wait for a message from the master
            MPI_Recv(&message, sizeof(ChessSearchState), MPI_PACKED, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
                     &status);

            if (status.MPI_TAG == tagFinished) {
                break;
            } else if (status.MPI_TAG == tagWork) {
                bestDepth = message.maxDepth;
                bestMoves = (int *) malloc(sizeof(int) * MOVES_SIZE_MAX);

                int statesCount = 4 * 100;
                queue = createQueue(statesCount);
                bfsChessboard(queue, &message);

#pragma omp parallel for schedule(dynamic)
                for (int i = 0; i < statesCount - 1; i++) {
                    dfsChessboard(queue->array[i]);
                }

                ChessSearchResult chessSearchResult;
                chessSearchResult.depth = bestDepth;
                copyIntArray(bestMoves, chessSearchResult.moves, MOVES_SIZE_MAX);

                MPI_Send(&chessSearchResult, sizeof(ChessSearchResult), MPI_PACKED, 0, tagFinished, MPI_COMM_WORLD);

                free(queue->array);
                free(queue);

                free(bestMoves);
            }
        }
    }

    MPI_Finalize();

    return 0;
}